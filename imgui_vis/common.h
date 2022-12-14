#pragma once

struct Log {
    stringstream s;

    Log() { s = stringstream(); }
    Log& clear() { s = stringstream(); return *this; }
};

template <class T>
Log& operator<<(Log& l, const T& other) {
    l.s << other;
    return l;
}

Log msg;
int S = 10;
float T = 0.01;
int optSeconds = 600;
bool optRunning;
bool hardMove;
bool regionOpt = true;
bool hardRects;
int drawR1, drawR2, drawC1, drawC2;
int hardIters = 5000;
int RS = 10;