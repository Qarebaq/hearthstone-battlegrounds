#include"Board.h"
#include<iostream>
#include<algorithm>
using namespace std;



void Board::addMinion(Minion *m){

    if(minions.size()< maxMinions){
        minions.push_back(m);
        cout<<m->name<<" added to the board "<<endl;
    
    }

    else{

        cout<<"Board is full! Cannot add more minions."<<endl;
    }

}



// برای این تابع اومدم یه وکتور دیگه درست کردم و زنده ها رو ریختم توش و با وکتور قبلی جایگزین کردم
void Board::removeDead() {
    vector<Minion*> alive;

    for(int i=0;i<minions.size(); i++){
        if(minions[i]->health>0){
            alive.push_back(minions[i]);
        }
        else{
            delete minions[i];// پسرک اینجا اگر مینیون مرده بود حذف میکنیم فضا اش آزاد شه
        }

    }

    minions = std::move(alive);
}


//اولین مینیون زنده رو برمیگردونه برای نبرد
Minion *Board::getNextAttacker(){

    for(int i=0;i<minions.size();i++){
        if(minions[i]->health>0){
            return minions[i];
        }
    }

return nullptr; //     برای حالتی هست که هیچ مینیون زنده ای نبود
}



//این تابع صرفا مشخصات بورد رو چاپ میکنه کمک برای دیباگ هم هست
void Board::printBoard(){

    cout<<"Board: ";
        for (auto m : minions) {
        cout << m->name << "(" << m->attack << "/" << m->health << ") ";
    }
    cout << endl;


}

//Clear = ==== dellte all minions and clear vector
void Board::clear(){
    for(auto m : minions){
        delete m;
    }
    minions.clear();

}

//here is my destructor
Board::~Board(){
    for(auto m : minions){
        delete m;
    }
    minions.clear();
}