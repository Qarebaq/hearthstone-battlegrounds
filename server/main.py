import asyncio
import os
from concurrent.futures import ThreadPoolExecutor
import orjson
import websockets

# پسرک اینجا دارم من اون کدهای سی پ پ ای که زدیم رو ایمپورت میکنم تا استفاده کنیم
import bgbinding

# We use a small thread pool to offload blocking C++ calls from the asyncio event loop.
executor = ThreadPoolExecutor(max_workers=6)

# Keep track of connected websocket clients per match.
MATCH_CLIENTS: dict[str, set] = {}


async def broadcast(match_id: str, obj: dict) -> None:
    """
    Send a JSON-serialisable object to all clients connected to the given match.
    Stale connections are silently dropped.
    """
    data = orjson.dumps(obj)
    conns = MATCH_CLIENTS.get(match_id, set()).copy()
    for ws in conns:
        try:
            await ws.send(data)
        except Exception:
            # Remove failed connections
            MATCH_CLIENTS.get(match_id, set()).discard(ws)


async def handle(websocket, path) -> None:
    """
    Handle a single websocket connection.

    Protocol (incoming JSON messages):
      - {"type": "create_match", "num_players": 4}
      - {"type": "join_match",   "match_id": "<id>"}
      - {"type": "action",
         "match_id": "<id>",
         "player_index": 0,
         "action": { ... } }
      - {"type": "start_combat", "match_id": "<id>", "seed": optional_int }

    Server responds with JSON messages:
      - {"type": "match_created", "match_id": "<id>", "num_players": int}
      - {"type": "state", ...state...}
      - {"type": "ack"}
      - {"type": "combat_log", ...state_after_combat...}
      - {"type": "error", "msg": "<description>"}
    """
    match_id: str | None = None
    loop = asyncio.get_running_loop()
    try:
        async for raw in websocket:
            try:
                msg = orjson.loads(raw)
            except Exception:
                await websocket.send(
                    orjson.dumps({"type": "error", "msg": "invalid json"})
                )
                continue

            msg_type = msg.get("type")
            if msg_type == "create_match":
                # Create a new match via the C++ engine.
                num_players = int(msg.get("num_players", 4))
                created_json = await loop.run_in_executor(
                    executor, bgbinding.create_match_json, num_players
                )
                try:
                    created = orjson.loads(created_json)
                except Exception:
                    created = {"match_id": created_json}
                # Record the match ID and register this websocket.
                match_id = str(created.get("match_id"))
                MATCH_CLIENTS.setdefault(match_id, set()).add(websocket)
                await websocket.send(
                    orjson.dumps({"type": "match_created", **created})
                )
                # Send initial state to the creator.
                state_json = await loop.run_in_executor(
                    executor, bgbinding.get_state_json, match_id
                )
                try:
                    state = orjson.loads(state_json)
                except Exception:
                    state = {}
                await websocket.send(
                    orjson.dumps({"type": "state", **state})
                )
            elif msg_type == "join_match":
                mid = msg.get("match_id")
                if not mid:
                    await websocket.send(
                        orjson.dumps(
                            {"type": "error", "msg": "missing match_id"}
                        )
                    )
                    continue
                match_id = str(mid)
                MATCH_CLIENTS.setdefault(match_id, set()).add(websocket)
                # Send the current state to the joining client.
                state_json = await loop.run_in_executor(
                    executor, bgbinding.get_state_json, match_id
                )
                try:
                    state = orjson.loads(state_json)
                except Exception:
                    state = {}
                await websocket.send(
                    orjson.dumps({"type": "state", **state})
                )
            elif msg_type == "action":
                mid = msg.get("match_id") or match_id
                if not mid:
                    await websocket.send(
                        orjson.dumps(
                            {"type": "error", "msg": "missing match_id"}
                        )
                    )
                    continue
                mid = str(mid)
                player_index = msg.get("player_index")
                if player_index is None:
                    await websocket.send(
                        orjson.dumps(
                            {"type": "error", "msg": "missing player_index"}
                        )
                    )
                    continue
                try:
                    player_index = int(player_index)
                except Exception:
                    await websocket.send(
                        orjson.dumps(
                            {"type": "error", "msg": "invalid player_index"}
                        )
                    )
                    continue
                action_obj = msg.get("action", {})
                try:
                    action_json = orjson.dumps(action_obj).decode()
                except Exception:
                    await websocket.send(
                        orjson.dumps(
                            {"type": "error", "msg": "invalid action"}
                        )
                    )
                    continue
                # Push the action to the C++ engine.
                await loop.run_in_executor(
                    executor,
                    bgbinding.push_action_json,
                    mid,
                    player_index,
                    action_json,
                )
                # Acknowledge receipt.
                await websocket.send(orjson.dumps({"type": "ack"}))
                # Broadcast updated state to all clients.
                state_json = await loop.run_in_executor(
                    executor, bgbinding.get_state_json, mid
                )
                try:
                    state = orjson.loads(state_json)
                except Exception:
                    state = {}
                await broadcast(mid, {"type": "state", **state})
            elif msg_type == "start_combat":
                mid = msg.get("match_id") or match_id
                if not mid:
                    await websocket.send(
                        orjson.dumps(
                            {"type": "error", "msg": "missing match_id"}
                        )
                    )
                    continue
                mid = str(mid)
                seed = msg.get("seed")
                if seed is not None:
                    try:
                        seed_int = int(seed)
                    except Exception:
                        await websocket.send(
                            orjson.dumps(
                                {"type": "error", "msg": "invalid seed"}
                            )
                        )
                        continue
                else:
                    seed_int = 0
                # Trigger combat (currently returns state).
                combat_json = await loop.run_in_executor(
                    executor, bgbinding.start_combat_json, mid, seed_int
                )
                try:
                    combat_state = orjson.loads(combat_json)
                except Exception:
                    combat_state = {}
                # Broadcast combat result to all.
                await broadcast(mid, {"type": "combat_log", **combat_state})
            else:
                await websocket.send(
                    orjson.dumps(
                        {"type": "error", "msg": f"unknown type: {msg_type}"}
                    )
                )
    except Exception:
        # Connection closed or other error – silently ignore.
        pass
    finally:
        # Remove the websocket from match clients on disconnect.
        if match_id and websocket in MATCH_CLIENTS.get(match_id, set()):
            MATCH_CLIENTS[match_id].discard(websocket)
            if not MATCH_CLIENTS[match_id]:
                MATCH_CLIENTS.pop(match_id, None)
                # Optionally free resources if no clients remain.
                try:
                    loop.run_in_executor(
                        executor, bgbinding.destroy_match, match_id
                    )
                except Exception:
                    pass


async def main() -> None:
    """
    Start the websocket server.  The port can be configured via the PORT
    environment variable (default: 8765).
    """
    port = int(os.environ.get("PORT", "8765"))
    async with websockets.serve(handle, "", port):
        print(f"Server listening on port {port}")
        # Run forever until cancelled.
        await asyncio.Future()


if __name__ == "__main__":
    asyncio.run(main())