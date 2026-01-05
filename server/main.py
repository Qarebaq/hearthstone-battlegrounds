import asyncio
import os
from concurrent.futures import ThreadPoolExecutor
import orjson
import websockets

#پسرک اینجا دارم من اون کدهای سی پ پ ای که زدیم رو ایمپورت میکنم تا استفاده کنیم
import bgbinding


executor = ThreadPoolExecutor(max_workers=6)


MATCH_CLIENTS={}


async def broadcast(match_id: str ,obj):
    data = orjson.dumps(obj)
    conns = MATCH_CLIENTS.get(match_id , set()).copy()
    for ws in conns:
        try:
            await ws.send(data)
        except Exception:
            MATCH_CLIENTS.get(match_id , set()).discard(ws)


async def handle(websocket , path):
    """
    Handle a single websocket connection.
    Protocol (JSON messages):
      - {"type":"create_match", "num_players":4}
      - {"type":"join_match", "match_id":"..."}
      - {"type":"action", "match_id":"...", "player_index":0, "action":{...}}
      - {"type":"start_combat", "match_id":"...", "seed": optional}
    Server responds with JSON messages: match_created, state, ack, combat_log, error
    """
    match_id = None
    try:
        async for raw in websocket:
            try:
                msg = orjson.loads(raw)
            except Exception:
                await websocket.send(orjson.dumps({"type":"error","msg":"invalid json"}))
                continue

            t = msg.get("type")
            if t == "create_match":
                num_players = int(msg.get("num_players" , 4))

                created_json = bgbinding.create_match_json(num_players) ##NEED to debug