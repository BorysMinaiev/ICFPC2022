#pragma once

#include "common.h"

#include <iomanip>

constexpr int tColor = 1;
constexpr int tSplitPoint = 2;
constexpr int tSplitX = 3;
constexpr int tSplitY = 4;
constexpr int tMerge = 5;
constexpr int tSwap = 6;

struct RawBlock {
    string id;
    int blX, blY, trX, trY;
    int r, g, b, a;
};

struct Block {
    int r1, c1, r2, c2;
    Color color;
};

struct Costs {
    double splitLine, splitPoint, color, swap, merge;
};

int N, M;
vector<vector<Color>> colors, initialColors;
vector<RawBlock> rawBlocks;
vector<Block> coloredBlocks;
Costs costs;
bool running;

struct Input {
    int N, M;
    vector<vector<Color>> colors, initialColors;
    vector<RawBlock> rawBlocks;
    Costs costs;
};

struct Instruction {
    string id, oid;
    int type;
    int x, y;
    Color color;

    string text() const {
        char buf[128];
        if (type == tColor) {
            sprintf(buf, "color [%s] [%d, %d, %d, %d]", id.c_str(), color[0], color[1], color[2], color[3]);
        } else if (type == tSplitPoint) {
            sprintf(buf, "cut [%s] [%d, %d]", id.c_str(), x, y);
        } else if (type == tSplitX) {
            sprintf(buf, "cut [%s] [X] [%d]", id.c_str(), x);
        } else if (type == tSplitY) {
            sprintf(buf, "cut [%s] [Y] [%d]", id.c_str(), y);
        } else if (type == tMerge) {
            sprintf(buf, "merge [%s] [%s]", id.c_str(), oid.c_str());
        } else if (type == tSwap) {
            sprintf(buf, "swap [%s] [%s]", id.c_str(), oid.c_str());
        } else assert(false);
        return buf;
    }
};

Instruction ColorIns(const string& i, Color c) {
    Instruction res;
    res.type = tColor;
    res.id = i;
    res.color = c;
    return res;
}

Instruction SplitPointIns(const string& i, int x, int y) {
    Instruction res;
    res.type = tSplitPoint;
    res.id = i;
    res.x = x;
    res.y = y;
    return res;
}

Instruction SplitXIns(const string& i, int x) {
    Instruction res;
    res.type = tSplitX;
    res.id = i;
    res.x = x;
    return res;
}

Instruction SplitYIns(const string& i, int y) {
    Instruction res;
    res.type = tSplitY;
    res.id = i;
    res.y = y;
    return res;
}

Instruction MergeIns(const string& i1, string i2) {
    Instruction res;
    res.type = tMerge;
    res.id = i1;
    res.oid = i2;
    return res;
}

Instruction SwapIns(const string& i1, const string& i2) {
    Instruction res;
    res.type = tSwap;
    res.id = i1;
    res.oid = i2;
    return res;
}

struct Solution {
    double score;
    vector<Instruction> ins;

    void rotateClockwise() {
      int last_cut = -1;
      for (auto& i : ins) {
        if (i.type == tSplitX) {
          last_cut = i.type;
          i.type = tSplitY;
          i.y = N - i.x;
          continue;
        }
        if (i.type == tSplitY) {
          last_cut = i.type;
          i.type = tSplitX;
          i.x = i.y;
          continue;
        }
        if (i.type == tSplitPoint) {
          last_cut = i.type;
          swap(i.x, i.y);
          i.y = N - i.y;
          continue;
        }
        if (i.type == tColor) {
          if (last_cut == tSplitX) {
            i.id.back() ^= 1;
          }
          if (last_cut == tSplitPoint) {
            if (i.id.back() == '0') {
              i.id.back() = '3';
            } else {
              i.id.back() -= 1;
            }
          }
          continue;
        }
        if (i.type == tMerge) {
          if (last_cut == tSplitPoint && i.id.find('.') != string::npos) {
            if (i.id.back() == '0') {
              i.id.back() = '3';
            } else {
              i.id.back() -= 1;
            }
            if (i.oid.back() == '0') {
              i.oid.back() = '3';
            } else {
              i.oid.back() -= 1;
            }
          }
        }
      }
    }
};

struct Painter {
    int lastBlockId;
    int N, M;
    unordered_map<string, Block> blocks;
    vector<vector<Color>> clr;
    double opsScore;
    vector<Block> coloredBlocks;

    Painter() {}
    Painter(int n, int m, const vector<RawBlock>& rb) {
        cerr << "created painter with " << rb.size() << " initial blocks\n";
        lastBlockId = rb.size() - 1;
        opsScore = 0;
        N = n;
        M = m;
        Color c;
        c[0] = c[1] = c[2] = c[3] = 255;
        clr.assign(n, vector<Color>(m, c));
        for (const auto& b : rb) {
            c[0] = b.r; c[1] = b.g; c[2] = b.b; c[3] = b.a;
            for (int x = b.blX; x < b.trX; x++)
                for (int y = b.blY; y < b.trY; y++)
                    clr[y][x] = c;
            blocks[b.id] = Block{b.blY, b.blX, b.trY, b.trX, c};
        }
    }

    bool doColor(const string& i, Color c) {
        if (blocks.find(i) == blocks.end())
            return false;
        const auto& b = blocks[i];
        opsScore += round(costs.color * N * M / ((b.r2 - b.r1) * (b.c2 - b.c1)));
        for (int i = b.r1; i < b.r2; i++)
            for (int j = b.c1; j < b.c2; j++)
                clr[i][j] = c;
        coloredBlocks.push_back(b);
        coloredBlocks.back().color = c;
        return true;
    }

    bool doSplitX(const string& i, int x) {
        // cerr << "doSplitX " << i << " " << x << endl;
        if (blocks.find(i) == blocks.end())
            return false;
        const auto& b = blocks[i];
        // cerr << "this block is " << b.r1 << "," << b.c1 << " - " << b.r2 << "," << b.c2 << endl;
        if (x <= b.c1 || x >= b.c2) return false;
        opsScore += round(costs.splitLine * N * M / ((b.r2 - b.r1) * (b.c2 - b.c1)));
        Block left = b;
        Block right = b;
        blocks.erase(blocks.find(i));
        left.c2 = x;
        right.c1 = x;
        blocks[i + ".0"] = left;
        blocks[i + ".1"] = right;
        return true;
    }

    bool doSplitY(const string& i, int y) {
        // cerr << "doSplitY " << i << " " << y << endl;
        if (blocks.find(i) == blocks.end())
            return false;
        const auto& b = blocks[i];
        // cerr << "this block is " << b.r1 << "," << b.c1 << " - " << b.r2 << "," << b.c2 << endl;
        if (y <= b.r1 || y >= b.r2) return false;
        opsScore += round(costs.splitLine * N * M / ((b.r2 - b.r1) * (b.c2 - b.c1)));
        Block down = b;
        Block up = b;
        blocks.erase(blocks.find(i));
        down.r2 = y;
        up.r1 = y;
        blocks[i + ".0"] = down;
        blocks[i + ".1"] = up;
        // cerr << "new blocks: " << i + ".0" << " " << i + ".1" << endl;
        return true;
    }

    bool doSplitPoint(const string& i, int x, int y) {
        if (blocks.find(i) == blocks.end())
            return false;
        const auto& b = blocks[i];
        if (x <= b.c1 || x >= b.c2) return false;
        if (y <= b.r1 || y >= b.r2) return false;
        opsScore += round(costs.splitPoint * N * M / ((b.r2 - b.r1) * (b.c2 - b.c1)));
        Block b0 = b;
        Block b1 = b;
        Block b2 = b;
        Block b3 = b;
        blocks.erase(blocks.find(i));
        b0.r2 = y; b1.r2 = y;
        b3.r1 = y; b2.r1 = y;
        b3.c2 = x; b0.c2 = x;
        b1.c1 = x; b2.c1 = x;
        blocks[i + ".0"] = b0;
        blocks[i + ".1"] = b1;
        blocks[i + ".2"] = b2;
        blocks[i + ".3"] = b3;
        return true;
    }

    bool doMerge(const string& i1, const string& i2) {
        // cerr << "doMerge " << i1 << " " << i2 << endl;
        if (blocks.find(i1) == blocks.end() || blocks.find(i2) == blocks.end())
            return false;

        const auto& bu = blocks[i1];
        const auto& bv = blocks[i2];
        // cerr << bu.r1 << "," << bu.c1 << " - " << bu.r2 << "," << bu.c2 << endl;
        // cerr << bv.r1 << "," << bv.c1 << " - " << bv.r2 << "," << bv.c2 << endl;
        opsScore += round(costs.merge * N * M / max((bu.r2 - bu.r1) * (bu.c2 - bu.c1),
                                            (bv.r2 - bv.r1) * (bv.c2 - bv.c1)));
        Block nb;
        if (bu.r2 == bv.r1 || bu.r1 == bv.r2) {
            if (bu.c1 == bv.c1 && bu.c2 == bv.c2) {
                if (bu.r2 == bv.r1) {
                    nb = bu;
                    nb.r2 = bv.r2;
                } else {
                    nb = bv;
                    nb.r2 = bu.r2;
                }
            } else {
                return false;
            }
        }

        if (bu.c2 == bv.c1 || bu.c1 == bv.c2) {
            if (bu.r1 == bv.r1 && bu.r2 == bv.r2) {
                if (bu.c2 == bv.c1) {
                    nb = bu;
                    nb.c2 = bv.c2;
                } else {
                    nb = bv;
                    nb.c2 = bu.c2;
                }
            } else {
                return false;
            }
        }

        blocks.erase(blocks.find(i1));
        blocks.erase(blocks.find(i2));
        lastBlockId++;
        blocks[to_string(lastBlockId)] = nb;
        // cerr << "new block: " << lastBlockId << endl;
        return true;
    }

    bool doSwap(const string& i1, const string& i2) {
        return true;
        if (blocks.find(i1) == blocks.end() || blocks.find(i2) == blocks.end())
            return false;

        const auto& bu = blocks[i1];
        const auto& bv = blocks[i2];
        if (bu.r2 - bu.r1 != bv.r2 - bv.r1 || bu.c2 - bu.c1 != bv.c2 - bv.c1)
            return false;
        opsScore += round(costs.swap * N * M / max((bu.r2 - bu.r1) * (bu.c2 - bu.c1),
                                                   (bv.r2 - bv.r1) * (bv.c2 - bv.c1)));
        
        for (int i = 0; i < bu.r2 - bu.r1; i++)
            for (int j = 0; j < bu.c2 - bu.c1; j++)
                swap(clr[bu.r1 + i][bu.c1 + j], clr[bv.r1 + i][bv.c1 + j]);
        return true;
    }

    bool doInstruction(const Instruction& ins) {
        if (ins.type == tColor) {
            return doColor(ins.id, ins.color);
        } else if (ins.type == tSplitPoint) {
            return doSplitPoint(ins.id, ins.x, ins.y);
        } else if (ins.type == tSplitX) {
            return doSplitX(ins.id, ins.x);
        } else if (ins.type == tSplitY) {
            return doSplitY(ins.id, ins.y);
        } else if (ins.type == tMerge) {
            return doMerge(ins.id, ins.oid);
        } else if (ins.type == tSwap) {
            return doSwap(ins.id, ins.oid);
        } else return false;
    }

    int totalScore(const vector<vector<Color>>& targetColors) const {
        double res = 0;
        // cerr << "opsScore = " << opsScore << endl;
        // cerr << clr.size() << " " << targetColors.size() << endl;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++) {
                double d = 0;
                for (int q = 0; q < 4; q++)
                    d += sqr(clr[i][j][q] - targetColors[i][j][q]);
                res += sqrt(d);
            }
        return round(res * 0.005 + opsScore);
    }
};

void postprocess(Solution& res);

int totalNodes;
int visitedNodes;
unordered_map<ll, double> mg;
double f[410][410];

double colorCost(int a, int b) {
    return round(costs.color * N * M / (a * b));
}

double splitLineCost(int a, int b) {
    return round(costs.splitLine * N * M / (a * b));
}

double mergeCost(int S, int a, int b) {
    return round(costs.merge * N * M / (S * max(a, b)));
}

double opsCost(int r, int c) {
    if (r == 0 && c == 0) return costs.color;
    if (r == 0) return costs.splitLine + colorCost(N, M - c) + mergeCost(N, c, M - c);
    if (c == 0) return costs.splitLine + colorCost(N - r, M) + mergeCost(M, r, N - r);
    return costs.splitPoint + colorCost(N - r, M - c) + mergeCost(M - c, r, N - r) + mergeCost(c, r, N - r) + mergeCost(N, c, M - c);
    // can check second order lul
}

double getG(int r1, int c1, int r2, int c2) {
    if (r1 == N || c1 == M) return 0;
    ll key = ((r1 * M + c1) * ll(N) + r2) * ll(M) + c2;
    if (mg.find(key) != mg.end()) {
        return mg[key];
    }

    double res = 1e9;
    double oc = opsCost(r1, c1);
    if (r2 == N) {
        Color sum;
        for (int q = 0; q < 4; q++) sum[q] = 0;
        int total = 0;

        for (int r = r1; r < r2; r++) {
            for (int c = c1; c < c2; c++) {
                for (int q = 0; q < 4; q++) sum[q] += colors[r][c][q];
                total++;
            }

            if ((r + 1) % S == 0) {
                Color avg;
                for (int q = 0; q < 4; q++)
                    avg[q] = sum[q] / total;
                double colorPenalty = 0;
                double cur = oc + getG(r + 1, c1, N, c2);
                for (int qr = r1; qr <= r; qr++)
                    for (int c = c1; c < c2; c++) {
                        double ssq = 0;
                        for (int q = 0; q < 4; q++)
                            ssq += sqr(avg[q] - colors[qr][c][q]);
                        colorPenalty += sqrt(ssq);
                        // if (cur + colorPenalty * 0.005 > res) {
                        //     break;
                        // }
                    }
                cur += colorPenalty * 0.005;
                if (cur < res) {
                    res = cur;
                    // store answer
                }
            }
        }
    }
    if (c2 == M) {
        Color sum;
        for (int q = 0; q < 4; q++) sum[q] = 0;
        int total = 0;

        for (int c = c1; c < c2; c++) {
            for (int r = r1; r < r2; r++) {
                for (int q = 0; q < 4; q++) sum[q] += colors[r][c][q];
                total++;
            }

            if ((c + 1) % S == 0) {
                Color avg;
                for (int q = 0; q < 4; q++)
                    avg[q] = sum[q] / total;
                double colorPenalty = 0;
                double cur = oc + getG(r1, c + 1, r2, M);
                for (int r = r1; r < r2; r++)
                    for (int qc = c1; qc <= c; qc++) {
                        double ssq = 0;
                        for (int q = 0; q < 4; q++)
                            ssq += sqr(avg[q] - colors[r][qc][q]);
                        colorPenalty += sqrt(ssq);
                        // if (cur + colorPenalty * 0.005 > res) {
                        //     break;
                        // }
                    }
                cur += colorPenalty * 0.005;
                if (cur < res) {
                    res = cur;
                    // store answer
                }
            }
        }
    }


    return mg[key] = res;
}

void solveDP2() {
    if (N % S != 0) {
        cerr << "N % S != 0\n";
        return;
    }
    auto start_time = Time::now();
    auto GetTime = [&]() {
      auto cur_time = Time::now();
      std::chrono::duration<double> fs = cur_time - start_time;
      return std::chrono::duration_cast<chrono_ms>(fs).count() * 0.001;
    };
    mg.clear();
    memset(f, 0, sizeof(f));
    for (int r = N - S; r >= 0; r -= S) {
        msg.clear() << "Running on row " << r << "...\n";
        cerr << r << " " << GetTime() << "s\n";
        for (int c = M - S; c >= 0; c -= S) {
            f[r][c] = 1e9;
            for (int c2 = c + S; c2 <= M; c2 += S) {
                f[r][c] = min(f[r][c], f[r][c2] + getG(r, c, N, c2));
            }
            for (int r2 = r + S; r2 <= N; r2 += S) {
                f[r][c] = min(f[r][c], f[r2][c] + getG(r, c, r2, M));
            }
        }
    }
    msg.clear() << "Result: " << f[0][0] << "\n";
}

int dp[20100][20100];
int aux[201][201];

int mode = 0;

vector<pair<int, int>> dp_corners;

pair<Solution, int> getTwoStepMerge(int blocksPerSide, int blockSize, int lines) {
    Solution res;
    int bid = 0;
    int nextBlockId = rawBlocks.size();
    res.score = 0;
    vector<int> vertBlocks;
    for (int i = 0; i < lines; i++) {
        int prevBlockId = bid;
        bid++;
        for (int j = 1; j < blocksPerSide; j++) {
            res.ins.push_back(MergeIns(to_string(prevBlockId), to_string(bid)));
            res.score += mergeCost(blockSize, blockSize * j, blockSize);
            prevBlockId = nextBlockId;
            nextBlockId++;
            bid++;
        }
        vertBlocks.push_back(prevBlockId);
    }
    int prevBlockId = vertBlocks[0];
    for (size_t i = 1; i < vertBlocks.size(); i++) {
        res.ins.push_back(MergeIns(to_string(prevBlockId), to_string(vertBlocks[i])));
        res.score += mergeCost(N, blockSize * i, blockSize);
        prevBlockId = nextBlockId;
        nextBlockId++;
    }

    string curId = to_string(nextBlockId - 1);
    vector<string> horIds;
    for (int i = 1; i < blocksPerSide; i++) {
        res.ins.push_back(SplitYIns(curId, i * N / blocksPerSide));
        res.score += splitLineCost(lines * blockSize, (blocksPerSide - i + 1) * blockSize);
        horIds.push_back(curId + ".0");
        curId += ".1";
    }
    horIds.push_back(curId);
    assert((int)horIds.size() == blocksPerSide);

    for (int i = lines; i < blocksPerSide; i++) {
        for (int j = 0; j < blocksPerSide; j++) {
            res.ins.push_back(MergeIns(horIds[j], to_string(bid)));
            res.score += mergeCost(blockSize, blockSize * i, blockSize);
            horIds[j] = to_string(nextBlockId);
            nextBlockId++;
            bid++;
        }
    }

    string prevHorId = horIds[0];
    for (int i = 1; i < blocksPerSide; i++) {
        res.ins.push_back(MergeIns(horIds[i], prevHorId));
        res.score += mergeCost(N, blockSize * i, blockSize);
        prevHorId = to_string(nextBlockId);
        nextBlockId++;
    }

    return {res, nextBlockId - 1};
}

pair<Solution, int> getThreeStepMerge(int blocksPerSide, int blockSize, int lines1, int lines2) {
    Solution res;
    int bid = 0;
    int nextBlockId = rawBlocks.size();
    res.score = 0;
    vector<int> vertBlocks;
    for (int i = 0; i < lines1; i++) {
        int prevBlockId = bid;
        bid++;
        for (int j = 1; j < blocksPerSide; j++) {
            res.ins.push_back(MergeIns(to_string(prevBlockId), to_string(bid)));
            res.score += mergeCost(blockSize, blockSize * j, blockSize);
            prevBlockId = nextBlockId;
            nextBlockId++;
            bid++;
        }
        vertBlocks.push_back(prevBlockId);
    }
    int prevBlockId = vertBlocks[0];
    for (size_t i = 1; i < vertBlocks.size(); i++) {
        res.ins.push_back(MergeIns(to_string(prevBlockId), to_string(vertBlocks[i])));
        res.score += mergeCost(N, blockSize * i, blockSize);
        prevBlockId = nextBlockId;
        nextBlockId++;
    }

    string curId = to_string(nextBlockId - 1);
    vector<string> horIds;
    for (int i = 1; i <= lines2; i++) {
        res.ins.push_back(SplitYIns(curId, i * N / blocksPerSide));
        res.score += splitLineCost(lines1 * blockSize, (blocksPerSide - i + 1) * blockSize);
        horIds.push_back(curId + ".0");
        curId += ".1";
    }
    auto rest1 = curId;
    assert((int)horIds.size() == lines2);
 
    for (int i = lines1; i < blocksPerSide; i++) {
        for (int j = 0; j < lines2; j++) {
            bid = i * blocksPerSide + j;
            res.ins.push_back(MergeIns(horIds[j], to_string(bid)));
            res.score += mergeCost(blockSize, blockSize * i, blockSize);
            horIds[j] = to_string(nextBlockId);
            nextBlockId++;
        }
    }

    string prevHorId = horIds[0];
    for (int i = 1; i < lines2; i++) {
        res.ins.push_back(MergeIns(horIds[i], prevHorId));
        res.score += mergeCost(N, blockSize * i, blockSize);
        prevHorId = to_string(nextBlockId);
        nextBlockId++;
    }

    curId = prevHorId;
    vector<string> verIds(blocksPerSide);
    for (int i = blocksPerSide - 1; i >= lines1; i--) {
        res.ins.push_back(SplitXIns(curId, i * N / blocksPerSide));
        res.score += splitLineCost(lines2 * blockSize, (i + 1) * blockSize);
        verIds[i] = curId + ".1";
        curId += ".0";
    }
    auto rest2 = curId;

    for (int i = lines1; i < blocksPerSide; i++) {
        for (int j = lines2; j < blocksPerSide; j++) {
            bid = i * blocksPerSide + j;
            res.ins.push_back(MergeIns(verIds[i], to_string(bid)));
            res.score += mergeCost(blockSize, blockSize * j, blockSize);
            verIds[i] = to_string(nextBlockId);
            nextBlockId++;
        }
    }

    string prevVerId = verIds.back();
    for (int i = blocksPerSide - 2; i >= lines1; i--) {
        res.ins.push_back(MergeIns(verIds[i], prevVerId));
        res.score += mergeCost(N, blockSize * (blocksPerSide - 1 - i), blockSize);
        prevVerId = to_string(nextBlockId);
        nextBlockId++;
    }

    auto rest3 = prevVerId;

    res.ins.push_back(MergeIns(rest1, rest2));
    res.score += mergeCost(lines1 * blockSize, lines2 * blockSize, (blocksPerSide - lines2) * blockSize);
    nextBlockId++;

    res.ins.push_back(MergeIns(to_string(nextBlockId - 1), rest3));
    res.score += mergeCost(N, lines1 * blockSize, (blocksPerSide - lines1) * blockSize);
    nextBlockId++;

    return {res, nextBlockId - 1};
}

int mergeCost(int a, int b) {
    return round(costs.merge * N * N / max(a, b));
}

int splitLineCost(int s) {
    return round(costs.splitLine * N * N / s);
}

pair<Solution, int> linesMerge() {
    const int B = round(sqrt(rawBlocks.size()));
    const int BS = N / B;
    const int BSq = BS * BS;
    assert(N == M);

    vector<vector<string>> blocks(B, vector<string>(B));
    for (int i = 0; i < B; i++)
        for (int j = 0; j < B; j++)
            blocks[i][j] = to_string(i + j * B);

    Solution res;
    res.score = 0;
    int nextBlockId = B * B;

    auto getSize = [&](const string& ii) {
        int cnt = 0;
        for (int i = 0; i < B; i++)
            for (int j = 0; j < B; j++)
                cnt += blocks[i][j] == ii;
        return cnt * BSq;
    };

    auto makeMerge = [&](string i1, string i2) {
        res.ins.push_back(MergeIns(i1, i2));
        res.score += mergeCost(getSize(i1), getSize(i2));
        string sid = to_string(nextBlockId);
        nextBlockId++;
        for (int i = 0; i < B; i++)
            for (int j = 0; j < B; j++)
                if (blocks[i][j] == i1 || blocks[i][j] == i2)
                    blocks[i][j] = sid;
    };

    auto makeSplitX = [&](string ii, int val) {
        res.ins.push_back(SplitXIns(ii, val * BS));
        res.score += splitLineCost(getSize(ii));
        string id1 = ii + ".0";
        string id2 = ii + ".1";
        // cerr << blocks[1][2] << " " << ii << " " << (blocks[1][2] == ii) << " B = " << B << endl;
        for (int i = 0; i < B; i++)
            for (int j = 0; j < B; j++) 
                if (blocks[i][j] == ii) {
                    // cerr << "setting " << i << " " << j << endl;
                    if (j < val) {
                        // cerr << "setting " << i << " " << j << " to " << id1 << endl;
                        blocks[i][j] = id1;
                    }
                    else {
                        // cerr << "setting " << i << " " << j << " to " << id2 << endl;
                        blocks[i][j] = id2;
                    }
                }
    };

    cerr << "! " << res.score << endl;

    auto makeSplitY = [&](string ii, int val) {
        res.ins.push_back(SplitYIns(ii, val * BS));
        res.score += splitLineCost(getSize(ii));
        string id1 = ii + ".0";
        string id2 = ii + ".1";
        for (int i = 0; i < B; i++)
            for (int j = 0; j < B; j++)
                if (blocks[i][j] == ii) {
                    if (i < val)
                        blocks[i][j] = id1;
                    else
                        blocks[i][j] = id2;
                }
    };

    for (int i = 2; i < B; i++) {
        makeMerge(blocks[i][0], blocks[i-1][0]);
        cerr << "! " << res.score << endl;
    }
    for (int i = 2; i < B; i++) {
        makeMerge(blocks[0][i], blocks[0][i-1]);
    }

    cerr << "! " << res.score << endl;

    for (int w = 1; w < B - 1; w++) {
        makeSplitX(blocks[w-1][w], w + 1);
        for (int i = w; i < B; i++) {
            makeMerge(blocks[i][w], blocks[i-1][w]);
        }
        makeSplitY(blocks[w][w], w);
        makeMerge(blocks[w][w], blocks[w][w-1]);
        makeMerge(blocks[w-1][w], blocks[w-1][w-1]);

        cerr << "! " << w << res.score << endl;
        
        makeSplitY(blocks[w][w], w + 1);
        for (int j = w + 1; j < B; j++)
            makeMerge(blocks[w][j], blocks[w][j-1]);

        // for (int i = 0; i < B; i++) {
        //     for (int j = 0; j < B; j++)
        //         cerr << std::setw(7) << blocks[i][j];
        //     fprintf(stderr, "\n");
        // }
        
        // cerr << "makeSplitX " << w << "," << w << " " << w + 1 << endl;
        makeSplitX(blocks[w][w], w + 1);

        // for (int i = 0; i < B; i++) {
        //     for (int j = 0; j < B; j++)
        //         cerr << std::setw(7) << blocks[i][j];
        //     fprintf(stderr, "\n");
        // }
        // cerr << "--------------------------------------------------------------\n";

        makeMerge(blocks[w][w], blocks[w-1][w]);
        makeMerge(blocks[w][w+1], blocks[w-1][w+1]);
        cerr << "! last " << w << res.score << endl;
    }

    makeMerge(blocks[B-1][B-1], blocks[B-2][B-1]);
    makeMerge(blocks[B-1][B-2], blocks[B-2][B-2]);
    makeMerge(blocks[B-1][B-1], blocks[B-1][B-2]);

    // for (const auto& i : res.ins)
    //     cerr << i.text() << endl;

    cerr << "lines merge score: " << res.score << endl;
    return {res, nextBlockId - 1};
}

pair<Solution, int> dpMerge() {
    const int B = round(sqrt(rawBlocks.size()));
    const int BS = N / B;
    const int BSq = BS * BS;
    assert(N == M);

    vector<vector<string>> blocks(B, vector<string>(B));
    for (int i = 0; i < B; i++)
        for (int j = 0; j < B; j++)
            blocks[i][j] = to_string(i + j * B);

    Solution res;
    res.score = 0;
    int nextBlockId = B * B;

    auto getSize = [&](const string& ii) {
        int cnt = 0;
        for (int i = 0; i < B; i++)
            for (int j = 0; j < B; j++)
                cnt += blocks[i][j] == ii;
        return cnt * BSq;
    };

    auto makeMerge = [&](string i1, string i2) {
        res.ins.push_back(MergeIns(i1, i2));
        res.score += mergeCost(getSize(i1), getSize(i2));
        string sid = to_string(nextBlockId);
        nextBlockId++;
        for (int i = 0; i < B; i++)
            for (int j = 0; j < B; j++)
                if (blocks[i][j] == i1 || blocks[i][j] == i2)
                    blocks[i][j] = sid;
    };

    auto makeSplitX = [&](string ii, int val) {
        res.ins.push_back(SplitXIns(ii, val * BS));
        res.score += splitLineCost(getSize(ii));
        string id1 = ii + ".0";
        string id2 = ii + ".1";
        for (int i = 0; i < B; i++)
            for (int j = 0; j < B; j++) 
                if (blocks[i][j] == ii) {
                    if (j < val) {
                        blocks[i][j] = id1;
                    }
                    else {
                        blocks[i][j] = id2;
                    }
                }
    };

    cerr << "! " << res.score << endl;

    auto makeSplitY = [&](string ii, int val) {
        res.ins.push_back(SplitYIns(ii, val * BS));
        res.score += splitLineCost(getSize(ii));
        string id1 = ii + ".0";
        string id2 = ii + ".1";
        for (int i = 0; i < B; i++)
            for (int j = 0; j < B; j++)
                if (blocks[i][j] == ii) {
                    if (i < val)
                        blocks[i][j] = id1;
                    else
                        blocks[i][j] = id2;
                }
    };

    const int inf = (int) 1e9;
    vector<vector<long long>> f1(B + 1, vector<long long>(B + 1, inf));
    vector<vector<long long>> f2(B + 1, vector<long long>(B + 1, inf));
    vector<vector<pair<int, int>>> pr1(B + 1, vector<pair<int, int>>(B + 1));
    vector<vector<int>> pt1(B + 1, vector<int>(B + 1));
    vector<vector<pair<int, int>>> pr2(B + 1, vector<pair<int, int>>(B + 1));
    vector<vector<int>> pt2(B + 1, vector<int>(B + 1));
    f1[0][0] = f2[0][0] = 0;
    int best = inf;
    int bi = -1;
    int bj = -1;
    for (int i = 0; i <= B; i++) {
      for (int j = 0; j <= B; j++) {
//        cerr << i << " " << j << " " << f1[i][j] << " " << f2[i][j] << endl;
        if (i == B && j == B) {
          continue;
        }
        if (i == B && j < B) {
          if (f1[i][j] < best) {
            best = f1[i][j];
            bi = i;
            bj = j;
          }
          continue;
        }
        if (i < B && j == B) {
          if (f2[i][j] < best) {
            best = f2[i][j];
            bi = i;
            bj = j;
          }
          continue;
        }
        {
          for (int j2 = j + 1; j2 <= B; j2++) {
            long long ft = f1[i][j];
            if (i > 0 && j > 0) {
              ft += llround(7.0 * B * B / i / B);
            }
            for (int col = j; col < j2; col++) {
              if (i > 0 && col < B - 1) {
                ft += llround(7.0 * B * B / i / (B - col));
              }
              for (int row = i; row < B; row++) {
                if (row > 0) {
                  ft += llround(1.0 * B * B / row / 1);
                }
              }
            }
            if (i > 0 && j > 0) {
              ft += llround(1.0 * B * B / max(i, B - i) / j);
            }
            for (int col = j; col < j2; col++) {
              if (col > 0) {
                ft += llround(1.0 * B * B / B / col);
              }
            }
            if (ft < f2[i][j2]) {
              f2[i][j2] = ft;
              pr2[i][j2] = make_pair(i, j);
              pt2[i][j2] = 1;
            }
          }
        }
        {
          for (int j2 = j + 1; j2 <= B; j2++) {
            long long ft = f1[i][j];
            if (i > 0 && j2 < B) {
              ft += llround(7.0 * B * B / i / B);
            }
            for (int col = j2 - 1; col >= j; col--) {
              if (i > 0 && col > 0) {
                ft += llround(7.0 * B * B / i / (col + 1));
              }
              for (int row = i; row < B; row++) {
                if (row > 0) {
                  ft += llround(1.0 * B * B / row / 1);
                }
              }
            }
            if (i > 0 && j > 0) {
              ft += llround(1.0 * B * B / max(i, B - i) / j);
            }
            for (int col = j; col < j2; col++) {
              if (col > 0) {
                ft += llround(1.0 * B * B / B / col);
              }
            }
            if (ft < f2[i][j2]) {
              f2[i][j2] = ft;
              pr2[i][j2] = make_pair(i, j);
              pt2[i][j2] = 2;
            }
          }
        }
        {
          for (int i2 = i + 1; i2 <= B; i2++) {
            long long ft = f2[i][j];
            if (i > 0 && j > 0) {
              ft += llround(7.0 * B * B / B / i);
            }
            for (int row = i; row < i2; row++) {
              if (j > 0 && row < B - 1) {
                ft += llround(7.0 * B * B / (B - row) / j);
              }
              for (int col = j; col < B; col++) {
                if (col > 0) {
                  ft += llround(1.0 * B * B / col / 1);
                }
              }
            }
            if (i > 0 && j > 0) {
              ft += llround(1.0 * B * B / i / max(j, B - j));
            }
            for (int row = i; row < i2; row++) {
              if (row > 0) {
                ft += llround(1.0 * B * B / row / B);
              }
            }
            if (ft < f1[i2][j]) {
              f1[i2][j] = ft;
              pr1[i2][j] = make_pair(i, j);
              pt1[i2][j] = 1;
            }
          }
        }
        {
          for (int i2 = i + 1; i2 <= B; i2++) {
            long long ft = f2[i][j];
            if (j > 0 && i2 < B) {
              ft += llround(7.0 * B * B / B / j);
            }
            for (int row = i2 - 1; row >= i; row--) {
              if (j > 0 && row > 0) {
                ft += llround(7.0 * B * B / (row + 1) / j);
              }
              for (int col = j; col < B; col++) {
                if (col > 0) {
                  ft += llround(1.0 * B * B / col / 1);
                }
              }
            }
            if (i > 0 && j > 0) {
              ft += llround(1.0 * B * B / i / max(j, B - j));
            }
            for (int row = i; row < i2; row++) {
              if (row > 0) {
                ft += llround(1.0 * B * B / row / B);
              }
            }
            if (ft < f1[i2][j]) {
              f1[i2][j] = ft;
              pr1[i2][j] = make_pair(i, j);
              pt1[i2][j] = 2;
            }
          }
        }
      }
    }

    cerr << "best = " << best << endl;
    
    vector<pair<int, int>> path(1, make_pair(bi, bj));
    vector<int> helpers;
    int flag = (bi == B ? 1 : 2);
    while (path.back() != make_pair(0, 0)) {
      if (flag == 1) {
        helpers.push_back(pt1[path.back().first][path.back().second]);
        path.push_back(pr1[path.back().first][path.back().second]);
      } else {
        helpers.push_back(pt2[path.back().first][path.back().second]);
        path.push_back(pr2[path.back().first][path.back().second]);
      }
      flag = 3 - flag;
    }
    reverse(path.begin(), path.end());
    reverse(helpers.begin(), helpers.end());

    for (int i = 0; i < (int) path.size(); i++) {
      cerr << path[i].first << " " << path[i].second << " " << (i < (int) helpers.size() ? helpers[i] : -1) << endl;
    }

    for (int it = 0; it < (int) path.size() - 1; it++) {
      int i = path[it].first;
      int j = path[it].second;
      int i2 = path[it + 1].first;
      int j2 = path[it + 1].second;
      cerr << i << " " << j << " " << i2 << " " << j2 << endl;
      if (j2 > j) {
        if (helpers[it] == 1) {
          if (i > 0 && j > 0) {
//            ft += llround(7.0 * B * B / i / B);
            makeSplitX(blocks[0][0], j);
          }
          for (int col = j; col < j2; col++) {
            if (i > 0 && col < B - 1) {
//              ft += llround(7.0 * B * B / i / (B - col));
              makeSplitX(blocks[0][B - 1], col + 1);
            }
          }
        } else {
          if (i > 0 && j2 < B) {
//            ft += llround(7.0 * B * B / i / B);
            makeSplitX(blocks[0][0], j2);
          }
          for (int col = j2 - 1; col >= j; col--) {
            if (i > 0 && col > 0) {
//              ft += llround(7.0 * B * B / i / (col + 1));
              makeSplitX(blocks[0][0], col);
            }
          }
        }
        for (int col = j; col < j2; col++) {
          for (int row = i; row < B; row++) {
            if (row > 0) {
//              ft += llround(1.0 * B * B / row / 1);
              makeMerge(blocks[row][col], blocks[row - 1][col]);
            }
          }
        }
        if (i > 0 && j > 0) {
//          ft += llround(1.0 * B * B / max(i, B - i) / j);
          makeMerge(blocks[0][0], blocks[B - 1][0]);
        }
        for (int col = j; col < j2; col++) {
          if (col > 0) {
//            ft += llround(1.0 * B * B / B / col);
            makeMerge(blocks[0][col], blocks[0][col - 1]);
          }
        }
      } else {
        if (helpers[it] == 1) {
          if (i > 0 && j > 0) {
//            ft += llround(7.0 * B * B / B / i);
            makeSplitY(blocks[0][0], i);
          }
          for (int row = i; row < i2; row++) {
            if (j > 0 && row < B - 1) {
//              ft += llround(7.0 * B * B / (B - row) / j);
              makeSplitY(blocks[B - 1][0], row + 1);
            }
          }
        } else {
          if (j > 0 && i2 < B) {
//            ft += llround(7.0 * B * B / B / j);
            makeSplitY(blocks[0][0], i2);
          }
          for (int row = i2 - 1; row >= i; row--) {
            if (j > 0 && row > 0) {
//              ft += llround(7.0 * B * B / (row + 1) / j);
              makeSplitY(blocks[0][0], row);
            }
          }
        }
        for (int row = i; row < i2; row++) {
          for (int col = j; col < B; col++) {
            if (col > 0) {
//              ft += llround(1.0 * B * B / col / 1);
              makeMerge(blocks[row][col], blocks[row][col - 1]);
            }
          }
        }
        if (i > 0 && j > 0) {
//          ft += llround(1.0 * B * B / i / max(j, B - j));
          makeMerge(blocks[0][0], blocks[0][B - 1]);
        }
        for (int row = i; row < i2; row++) {
          if (row > 0) {
//            ft += llround(1.0 * B * B / B / row);
            makeMerge(blocks[row][0], blocks[row - 1][0]);
          }
        }
      }
/*      for (int i = 0; i < B; i++) {
        for (int j = 0; j < B; j++) cerr << blocks[i][j] << " \n"[j == B - 1];
      }*/
    }


/*    for (const auto& i : res.ins)
        cerr << i.text() << endl;*/

    cerr << "lines merge score: " << res.score << endl;
    return {res, nextBlockId - 1};
}

pair<Solution, int> initialMerge() {
    return dpMerge();
//    return dpMerge();
    Solution res;
    int blocksPerSide = round(sqrt(rawBlocks.size()));
    assert(N == M);
    int blockSize = N / blocksPerSide;

    int bid = 0;
    int nextBlockId = rawBlocks.size();
    res.score = 0;
    vector<int> vertBlocks;
    for (int i = 0; i < blocksPerSide; i++) {
        int prevBlockId = bid;
        bid++;
        for (int j = 1; j < blocksPerSide; j++) {
            res.ins.push_back(MergeIns(to_string(prevBlockId), to_string(bid)));
            res.score += mergeCost(blockSize, blockSize * j, blockSize);
            prevBlockId = nextBlockId;
            nextBlockId++;
            bid++;
        }
        vertBlocks.push_back(prevBlockId);
    }
    int prevBlockId = vertBlocks[0];
    for (int i = 1; i < blocksPerSide; i++) {
        res.ins.push_back(MergeIns(to_string(prevBlockId), to_string(vertBlocks[i])));
        res.score += mergeCost(N, blockSize * i, blockSize);
        prevBlockId = nextBlockId;
        nextBlockId++;
    }

    cerr << "blocksPerSide: " << blocksPerSide << endl;
    for (int lines = 1; lines < blocksPerSide; lines++) {
        auto cur = getTwoStepMerge(blocksPerSide, blockSize, lines);
        if (cur.first.score < res.score) {
            cerr << "two-step-merge better with " << lines << " lines: " << res.score << " -> " << cur.first.score << endl;
            res = cur.first;
            nextBlockId = cur.second + 1;
        }
    }
    
    for (int lines1 = 1; lines1 < blocksPerSide; lines1++) {
        for (int lines2 = 1; lines2 < blocksPerSide; lines2++) {
            auto cur = getThreeStepMerge(blocksPerSide, blockSize, lines1, lines2);
            if (cur.first.score < res.score) {
                cerr << "three-step-merge better with " << lines1 << " and " << lines2 << " lines: " << res.score << " -> " << cur.first.score << endl;
                res = cur.first;
                nextBlockId = cur.second + 1;
            }
        }
    }
    
    return {res, nextBlockId - 1};
}


int PaintCost(int x, int y) {
  assert(x > 0 && y > 0);
  if (x == N && y == M) {
    return costs.color;
  }
  if (x == N) {
    return costs.splitLine + llround(costs.color * M / y) + llround(costs.merge * M / max(y, M - y));
  }
  if (y == M) {
    return costs.splitLine + llround(costs.color * N / x) + llround(costs.merge * N / max(x, N - x));
  }
  int ret = costs.splitPoint + llround(costs.color * (N * M) / (x * y));
  int cand1 = llround(costs.merge * (N * M) / (max(x, N - x) * y));
  cand1    += llround(costs.merge * (N * M) / (max(x, N - x) * (M - y)));
  cand1    += llround(costs.merge * M / max(y, M - y));
  int cand2 = llround(costs.merge * (N * M) / (x * max(y, M - y)));
  cand2    += llround(costs.merge * (N * M) / ((N - x) * max(y, M - y)));
  cand2    += llround(costs.merge * N / max(x, M - x));
  return ret + min(cand1, cand2);
}

void solveGena(int S, int mode) {
/*    if (S == -1) {
      S = ::S;
    }
    if (mode == -1) {
      mode = ::mode;
    }*/
    if (S < 2) {
      msg.clear() << "sorry, S must be at least 2";
      return;
    }
    if (N % S != 0 || M % S != 0) {
      msg.clear() << "sorry, N and M should be divisible by S";
      return;
    }
    auto start_time = Time::now();
    auto GetTime = [&]() {
      auto cur_time = Time::now();
      std::chrono::duration<double> fs = cur_time - start_time;
      return std::chrono::duration_cast<chrono_ms>(fs).count() * 0.001;
    };
    msg.clear() << "Running...";
    vector<vector<Color>> target_colors(N, vector<Color>(N));
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        target_colors[i][j] = colors[i][j];
      }
    }
    for (int rep = 0; rep < mode; rep++) {
      vector<vector<Color>> rotated(N, vector<Color>(N));
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          rotated[j][N - 1 - i] = target_colors[i][j];
        }
      }
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          target_colors[i][j] = rotated[i][j];
        }
      }
    }
    int n = N / S;
    int m = M / S;
    vector<vector<vector<int>>> pref(N + 1, vector<vector<int>>(M + 1, vector<int>(4)));
    for (int i = 0; i <= N; i++) {
      for (int j = 0; j <= M; j++) {
        for (int k = 0; k < 4; k++) {
          if (i == 0 || j == 0) {
            pref[i][j][k] = 0;
          } else {
            pref[i][j][k] = pref[i - 1][j][k] + pref[i][j - 1][k] - pref[i - 1][j - 1][k] + target_colors[i - 1][j - 1][k];
          }
        }
      }
    }
    const int MAX_D = 255 * 255 * 4;
    vector<double> SQRT(MAX_D + 1);
    for (int i = 0; i <= MAX_D; i++) {
      SQRT[i] = sqrt(i);
    }

    {
      assert(N == M);
      int tmp = 0;
      for (int xa = 0; xa < n; xa++) {
        for (int xb = xa + 1; xb <= n; xb++) {
          aux[xa][xb] = tmp++;
        }
      }
    }
    int zzseed = time(0);
    mt19937 rng(zzseed);
    for (int xa = n - 1; xa >= 0; xa--) {
      auto time_elapsed = GetTime();
      msg.clear() << "n = " << n << ", xa = " << xa << ", time = " << time_elapsed << "s\n";
      for (int ya = m - 1; ya >= 0; ya--) {
        for (int xb = xa + 1; xb <= n; xb++) {
          for (int yb = ya + 1; yb <= m; yb++) {
            int ft = (int) 1e9;
            for (int x = xa + 1; x < xb; x++) {
              ft = min(ft, dp[aux[xa][x]][aux[ya][yb]] + dp[aux[x][xb]][aux[ya][yb]]);
            }
            for (int y = ya + 1; y < yb; y++) {
              ft = min(ft, dp[aux[xa][xb]][aux[ya][y]] + dp[aux[xa][xb]][aux[y][yb]]);
            }
            int area = (xb - xa) * (yb - ya) * S * S;
            Color paint_into;
            for (int k = 0; k < 4; k++) {
              int sum = pref[yb * S][xb * S][k] - pref[ya * S][xb * S][k] - pref[yb * S][xa * S][k] + pref[ya * S][xa * S][k];
              paint_into[k] = (2 * sum + area) / (2 * area);
            }
            long long penalty = 1000 * PaintCost(N - xa * S, M - ya * S);
            if (penalty < ft) {
              double diff_est = 0;
              if (area >= S) {
                for (int y = ya * S; y < yb * S; y++) {
                  int x = xa * S + (int) (rng() % (xb * S - xa * S));
                  int sum_sq = 0;
                  for (int k = 0; k < 4; k++) {
                    sum_sq += sqr(target_colors[y][x][k] - paint_into[k]);
                  }
                  diff_est += SQRT[sum_sq];
                }
              }
              diff_est *= xb * S - xa * S;
              if (penalty + llround(diff_est * 5 * 0.8) < ft) {
                double diff = 0;
                for (int y = ya * S; y < yb * S; y++) {
                  for (int x = xa * S; x < xb * S; x++) {
                    int sum_sq = 0;
                    for (int k = 0; k < 4; k++) {
                      sum_sq += sqr(target_colors[y][x][k] - paint_into[k]);
                    }
                    diff += SQRT[sum_sq];
                  }
                  if (penalty + llround(diff * 5) >= ft) {
                    break;
                  }
                }
                penalty += llround(diff * 5);
                ft = min(ft, (int) penalty);
              }
            }
            dp[aux[xa][xb]][aux[ya][yb]] = ft;
          }
        }
      }
    }
    msg << "dp = " << dp[aux[0][n]][aux[0][m]] / 1000 << "\n";
    vector<pair<array<int, 4>, Color>> rects;
    vector<vector<int>> rect_id(n, vector<int>(m, -1));
    function<void(int, int, int, int)> Reconstruct = [&](int xa, int ya, int xb, int yb) {
      int ft = dp[aux[xa][xb]][aux[ya][yb]];
      for (int x = xa + 1; x < xb; x++) {
        if (dp[aux[xa][x]][aux[ya][yb]] + dp[aux[x][xb]][aux[ya][yb]] == ft) {
          Reconstruct(xa, ya, x, yb);
          Reconstruct(x, ya, xb, yb);
          return;
        }
      }
      for (int y = ya + 1; y < yb; y++) {
        if (dp[aux[xa][xb]][aux[ya][y]] + dp[aux[xa][xb]][aux[y][yb]] == ft) {
          Reconstruct(xa, ya, xb, y);
          Reconstruct(xa, y, xb, yb);
          return;
        }
      }
      int area = (xb - xa) * (yb - ya) * S * S;
      Color paint_into;
      for (int k = 0; k < 4; k++) {
        int sum = pref[yb * S][xb * S][k] - pref[ya * S][xb * S][k] - pref[yb * S][xa * S][k] + pref[ya * S][xa * S][k];
        paint_into[k] = (2 * sum + area) / (2 * area);
      }
      for (int x = xa; x < xb; x++) {
        for (int y = ya; y < yb; y++) {
          rect_id[x][y] = (int) rects.size();
        }
      }
      rects.emplace_back(array<int, 4>{xa, ya, xb, yb}, paint_into);
    };
    Reconstruct(0, 0, n, m);
    auto [res_pref, idx] = initialMerge();
    Solution res;
    res.score = res_pref.score;
    res.score += dp[aux[0][n]][aux[0][m]] / 1000;
    int rect_cnt = (int) rects.size();
    vector<vector<int>> graph(rect_cnt);
    vector<int> indegree(rect_cnt);
    auto AddEdge = [&](int i, int j) {
      if (i != j) {
        graph[i].push_back(j);
        indegree[j] += 1;
      }
    };
    for (int x = 0; x < n; x++) {
      for (int y = 0; y < m - 1; y++) {
        if (mode & 0) {
          AddEdge(rect_id[x][y + 1], rect_id[x][y]);
        } else {
          AddEdge(rect_id[x][y], rect_id[x][y + 1]);
        }
      }
    }
    for (int x = 0; x < n - 1; x++) {
      for (int y = 0; y < m; y++) {
        if (mode & 0) {
          AddEdge(rect_id[x + 1][y], rect_id[x][y]);
        } else {
          AddEdge(rect_id[x][y], rect_id[x + 1][y]);
        }
      }
    }
    vector<int> que;
    for (int i = 0; i < rect_cnt; i++) {
      if (indegree[i] == 0) {
        que.push_back(i);
      }
    }
    for (int b = 0; b < (int) que.size(); b++) {
      for (int u : graph[que[b]]) {
        if (--indegree[u] == 0) {
          que.push_back(u);
        }
      }
    }
    assert((int) que.size() == rect_cnt);
    auto Compare = [&](int x, int y) {
      int cand1 = llround(costs.merge * (n * m) / (max(x, n - x) * y));
      cand1    += llround(costs.merge * (n * m) / (max(x, n - x) * (m - y)));
      cand1    += llround(costs.merge * m / max(y, m - y));
      int cand2 = llround(costs.merge * (n * m) / (x * max(y, m - y)));
      cand2    += llround(costs.merge * (n * m) / ((n - x) * max(y, m - y)));
      cand2    += llround(costs.merge * n / max(x, m - x));
      return cand1 < cand2;
    };
    dp_corners.clear();
    for (int it = 0; it < rect_cnt; it++) {
      int i = que[it];
      int xa = rects[i].first[0];
      int ya = rects[i].first[1];
      int xb = rects[i].first[2];
      int yb = rects[i].first[3];
      dp_corners.emplace_back(xa * S, ya * S);
      Color paint_into = rects[i].second;
      if (mode >= 0) {
        if (xa == 0 && ya == 0) {
          res.ins.push_back(ColorIns(to_string(idx), paint_into));
        }
        if (xa == 0 && ya > 0) {
          res.ins.push_back(SplitYIns(to_string(idx), ya * S));
          res.ins.push_back(ColorIns(to_string(idx) + ".1", paint_into));
          res.ins.push_back(MergeIns(to_string(idx) + ".0", to_string(idx) + ".1"));
          idx += 1;
        }
        if (xa > 0 && ya == 0) {
          res.ins.push_back(SplitXIns(to_string(idx), xa * S));
          res.ins.push_back(ColorIns(to_string(idx) + ".1", paint_into));
          res.ins.push_back(MergeIns(to_string(idx) + ".0", to_string(idx) + ".1"));
          idx += 1;
        }
        if (xa > 0 && ya > 0) {
          res.ins.push_back(SplitPointIns(to_string(idx), xa * S, ya * S));
          res.ins.push_back(ColorIns(to_string(idx) + ".2", paint_into));
          if (Compare(n - xa, n - ya)) {
            res.ins.push_back(MergeIns(to_string(idx) + ".3", to_string(idx) + ".2"));
            res.ins.push_back(MergeIns(to_string(idx) + ".0", to_string(idx) + ".1"));
          } else {
            res.ins.push_back(MergeIns(to_string(idx) + ".3", to_string(idx) + ".0"));
            res.ins.push_back(MergeIns(to_string(idx) + ".2", to_string(idx) + ".1"));
          }
          res.ins.push_back(MergeIns(to_string(idx + 1), to_string(idx + 2)));
          idx += 3;
        }
      }
    }

    for (int rep = 0; rep < mode; rep++) {
      res.rotateClockwise();
    }
    res.ins.insert(res.ins.begin(), res_pref.ins.begin(), res_pref.ins.end());

    msg << "Duration: " << GetTime() << "s\n";
    postprocess(res);
}

void solveOpt() {
//    solveGena(10, 0);
    auto init_corners = dp_corners;
    auto start_time = Time::now();
    auto GetTime = [&]() {
      auto cur_time = Time::now();
      std::chrono::duration<double> fs = cur_time - start_time;
      return std::chrono::duration_cast<chrono_ms>(fs).count() * 0.001;
    };
    msg.clear() << "Running...\n";
    auto myColoredBlocks = coloredBlocks;
    vector<vector<Color>> target_colors(N, vector<Color>(N));
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        target_colors[i][j] = colors[i][j];
      }
    }
    int mode = 0;
    while (mode < 4) {
      bool ok = true;
      for (auto& block : myColoredBlocks) {
        if (block.r2 != N || block.c2 != N) {
          ok = false;
          break;
        }
      }
      if (ok) {
        break;
      }
      for (auto& block : myColoredBlocks) {
        swap(block.c1, block.r1);
        block.c1 = N - block.c1;
        swap(block.c2, block.r2);
        block.c2 = N - block.c2;
        swap(block.c1, block.c2);
      }
      vector<vector<Color>> rotated(N, vector<Color>(N));
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          rotated[j][N - 1 - i] = target_colors[i][j];
        }
      }
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          target_colors[i][j] = rotated[i][j];
        }
      }
      mode += 1;
    }
    cerr << "mode = " << mode << endl;
    assert(mode < 4);
    vector<vector<pair<int, int>>> top(N, vector<pair<int, int>>(N));
    vector<vector<list<pair<int, int>>::iterator>> iter(N, vector<list<pair<int, int>>::iterator>(N));
    vector<vector<list<pair<int, int>>>> cells(N, vector<list<pair<int, int>>>(N));
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        top[i][j] = make_pair(0, 0);
        cells[0][0].emplace_back(i, j);
        iter[i][j] = prev(cells[0][0].end());
      }
    }
    int n = N;
    int m = M;
    const int MAX_D = 255 * 255 * 4;
    vector<double> SQRT(MAX_D + 1);
    for (int i = 0; i <= MAX_D; i++) {
      SQRT[i] = sqrt(i);
    }
    vector<vector<int>> base_cost(N, vector<int>(N));
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        base_cost[i][j] = 1000 * PaintCost(N - i, N - j);
      }
    }
    vector<pair<int, int>> corners;
    vector<vector<int>> pos_in_corners(N, vector<int>(N, -1));
    auto Priority = [&](pair<int, int> x) {
      return pos_in_corners[x.first][x.second];
    };
    auto Choose = [&](pair<int, int> x, pair<int, int> y) {
      auto px = Priority(x);
      auto py = Priority(y);
      return (px > py ? x : y);
    };
    vector<vector<int>> cost(N, vector<int>(N, 0));
    vector<vector<Color>> paint_into(N, vector<Color>(N, {-1, -1, -1, -1}));
//    pos_in_corners[0][0] = 0;
//    corners.emplace_back(0, 0);
    int total = 0;
    auto Recalc = [&](int i, int j) {
      total -= cost[i][j];
      assert(top[i][j] == make_pair(i, j));
      assert(!cells[i][j].empty());
      if (1 || paint_into[i][j][0] == -1) {
        paint_into[i][j] = {0, 0, 0, 0};
        for (auto& cell : cells[i][j]) {
          for (int k = 0; k < 4; k++) {
            paint_into[i][j][k] += target_colors[cell.second][cell.first][k];
          }
        }
        int area = (int) cells[i][j].size();
        for (int k = 0; k < 4; k++) {
          paint_into[i][j][k] = (2 * paint_into[i][j][k] + area) / (2 * area);
        }
      }
      for (int rep = 0; rep < 5; rep++) {
        array<double, 4> aux = {0, 0, 0, 0};
        double sum_coeff = 0;
        for (auto& cell : cells[i][j]) {
          int sum_sq = 0;
          for (int k = 0; k < 4; k++) {
            sum_sq += sqr(target_colors[cell.second][cell.first][k] - paint_into[i][j][k]);
          }
          double coeff = 1.0 / max(1.0, SQRT[sum_sq]);
          sum_coeff += coeff;
          for (int k = 0; k < 4; k++) {
            aux[k] += target_colors[cell.second][cell.first][k] * coeff;
          }
        }
        auto old = paint_into[i][j];
        for (int k = 0; k < 4; k++) {
          paint_into[i][j][k] = llround(aux[k] / sum_coeff);
        }
        if (paint_into[i][j] == old) {
          break;
        }
      }
      cost[i][j] = base_cost[i][j];
      double diff = 0;
      for (auto& cell : cells[i][j]) {
        int sum_sq = 0;
        for (int k = 0; k < 4; k++) {
          sum_sq += sqr(target_colors[cell.second][cell.first][k] - paint_into[i][j][k]);
        }
        diff += SQRT[sum_sq];
      }
      while (true) {
        bool changed = false;
        for (int k = 0; k < 4; k++) {
          for (int delta = -1; delta <= 1; delta += 2) {
            paint_into[i][j][k] += delta;
            double new_diff = 0;
            for (auto& cell : cells[i][j]) {
              int sum_sq = 0;
              for (int k = 0; k < 4; k++) {
                sum_sq += sqr(target_colors[cell.second][cell.first][k] - paint_into[i][j][k]);
              }
              new_diff += SQRT[sum_sq];
            }
            if (new_diff < diff) {
              changed = true;
              diff = new_diff;
            } else {
              paint_into[i][j][k] -= delta;
            }
          }
        }
        if (!changed) {
          break;
        }
      }
      cost[i][j] += llround(diff * 5);
      total += cost[i][j];
    };
    Recalc(0, 0);
/*    auto ForceSetTop = [&](int i, int j, pair<int, int> new_top) {
      cells[top[i][j].first][top[i][j].second].erase(iter[i][j]);
      top[i][j] = new_top;
      cells[new_top.first][new_top.second].emplace_back(i, j);
      iter[i][j] = prev(cells[new_top.first][new_top.second].end());
    };*/
    auto ForceRecalcTop = [&](int i, int j) {
      assert(i > 0 || j > 0);
      auto new_top = (i == 0 ? top[i][j - 1] : (j == 0 ? top[i - 1][j] : Choose(top[i - 1][j], top[i][j - 1])));
      if (new_top == top[i][j]) {
        return false;
      }
      cells[top[i][j].first][top[i][j].second].erase(iter[i][j]);
      top[i][j] = new_top;
      cells[new_top.first][new_top.second].emplace_back(i, j);
      iter[i][j] = prev(cells[new_top.first][new_top.second].end());
      return true;
    };
    auto RecalcTop = [&](int i, int j) {
      if (top[i][j] == make_pair(i, j)) {
        return false;
      }
      return ForceRecalcTop(i, j);
    };
    int zzseed = time(0);
    mt19937 rng(zzseed);
    uniform_real_distribution<double> urd(0, 1);
    auto AddCorner = [&](int i, int j, int where) {
      assert(top[i][j] != make_pair(i, j));
      if (where == -1) {
        int L = 0;
        int R = (int) corners.size();
        for (int it = 0; it < (int) corners.size(); it++) {
          if (corners[it].first <= i && corners[it].second <= j) {
            L = max(L, it + 1);
          }
          if (i <= corners[it].first && j <= corners[it].second) {
            R = min(R, it);
          }
        }
        assert(L <= R);
        where = L + (rng() % (R - L + 1));
      }
      for (int it = where; it < (int) corners.size(); it++) {
        pos_in_corners[corners[it].first][corners[it].second] += 1;
      }
      pos_in_corners[i][j] = where;
      corners.insert(corners.begin() + where, make_pair(i, j));
      set<pair<int, int>> to_recalc;
      to_recalc.emplace(i, j);
      to_recalc.insert(top[i][j]);
      cells[top[i][j].first][top[i][j].second].erase(iter[i][j]);
      top[i][j] = make_pair(i, j);
      cells[i][j].emplace_back(i, j);
      iter[i][j] = prev(cells[i][j].end());
      for (int ii = i; ii < N; ii++) {
        if (ii > i) {
          auto old = top[ii][j];
          if (!RecalcTop(ii, j)) {
            break;
          }
          to_recalc.insert(old);
        }
        for (int jj = j + 1; jj < N; jj++) {
          auto old = top[ii][jj];
          if (!RecalcTop(ii, jj)) {
            break;
          }
          to_recalc.insert(old);
        }
      }
      for (auto& corner : to_recalc) {
        Recalc(corner.first, corner.second);
      }
    };
    auto RemoveCorner = [&](int i, int j) {
      assert(top[i][j] == make_pair(i, j));
      assert(i > 0 || j > 0);
      int ps = pos_in_corners[i][j];
      for (int it = ps + 1; it < (int) corners.size(); it++) {
        pos_in_corners[corners[it].first][corners[it].second] -= 1;
      }
      pos_in_corners[i][j] = -1;
      corners.erase(corners.begin() + ps);
      set<pair<int, int>> to_recalc;
      ForceRecalcTop(i, j);
      to_recalc.insert(top[i][j]);
      for (int ii = i; ii < N; ii++) {
        if (ii > i) {
          if (top[ii][j] != make_pair(i, j) || !RecalcTop(ii, j)) {
            break;
          }
          to_recalc.insert(top[ii][j]);
        }
        for (int jj = j + 1; jj < N; jj++) {
          if (top[ii][jj] != make_pair(i, j) || !RecalcTop(ii, jj)) {
            break;
          }
          to_recalc.insert(top[ii][jj]);
        }
      }
      for (auto& corner : to_recalc) {
        Recalc(corner.first, corner.second);
      }
      total -= cost[i][j];
      cost[i][j] = 0;
    };
    auto [res_pref, idx] = initialMerge();
    Solution res;
    res.score = res_pref.score;
    cerr << "total = " << res.score + total << endl;
    for (auto& block : myColoredBlocks) {
      if ((block.r1 > 0 || block.c1 > 0) && top[block.c1][block.r1] != make_pair(block.c1, block.r1)) {
        AddCorner(block.c1, block.r1, (int) corners.size());
      }
    }
    cerr << "total = " << res.score + total << endl;
//    for (int i = 0; i < N; i += 40) for (int j = 0; j < N; j += 40) if (i > 0 || j > 0) AddCorner(i, j);
    int qit = 0;
    #define wlog(operationType) msg.clear() << "it " << it << "|" << qit << " [" << operationType << "] cnt: " << corners.size() \
            << ", total: " << res.score + total / 1000 << " (" << res.score << "+" << total / 1000.0 << "), best: " << res.score + best_total / 1000 << ", time: " << GetTime() << + "s\n"
    #define setlocal localTries = 100; localI = i; localJ = j;
    int localTries = 0;
    int localI = -1, localJ = -1;
    vector<pair<pair<int, int>, Color>> rects;
    rects.emplace_back(make_pair(0, 0), paint_into[0][0]);
    for (auto& p : corners) {
      rects.emplace_back(p, paint_into[p.first][p.second]);
    }
    int best_total = total;

    auto optimizeHard = [&](int r1, int c1, int r2, int c2, int maxIters) {
        int start_total = total;
        vector<pair<pair<int, int>, int>> save;
        for (int i = r1; i < r2; i++)
            for (int j = c1; j < c2; j++)
                if (top[i][j] == make_pair(i, j)) {
                    save.emplace_back(top[i][j], pos_in_corners[i][j]);
                    RemoveCorner(i, j);
                }

        if (save.empty()) return false;

        for (int i = r1; i < r2; i++)
            for (int j = c1; j < c2; j++)
                if (rng() % 7 == 0) {
                    AddCorner(i, j, -1);
                }

        drawR1 = r1;
        drawR2 = r2;
        drawC1 = c1;
        drawC2 = c2;
        cerr << "optimizeHard " << r1 << "," << c1 << " - " << r2 << "," << c2 << ", " << save.size() << " removed:\n";
        cerr << start_total / 1000.0 << " -> " << total / 1000.0;
        // best_total = 1e9;

        for (int it = 0; it < maxIters && optRunning; it++) {
          if (total < best_total) {
            best_total = total;
            rects.clear();
            rects.emplace_back(make_pair(0, 0), paint_into[0][0]);
            for (auto& p : corners) {
              rects.emplace_back(p, paint_into[p.first][p.second]);
            }
          }

          T = (1 - double(it) / maxIters) * (1 - double(it) / maxIters) * (1 - double(it) / maxIters);

          vector<int> cidsInRegion;
          for (size_t id = 0; id < corners.size(); id++)
            if (r1 <= corners[id].first && corners[id].first < r2 &&
                c1 <= corners[id].second && corners[id].second < c2) {
                cidsInRegion.push_back(id);
            }
          // cerr << cidsInRegion.size() << "...";

          if (corners.size() >= 2 && !cidsInRegion.empty()) {
            int id = cidsInRegion[rng() % (int)cidsInRegion.size()];
            int i = corners[id].first;
            int j = corners[id].second;
            bool bad = false;
            if (!bad) {
                auto old_total = total;
                RemoveCorner(i, j);
                AddCorner(i, j, -1);
                if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
                  wlog("SWP");
                } else {
                  RemoveCorner(i, j);
                  AddCorner(i, j, id);
                }
            }
          }
          if (!corners.empty() && !cidsInRegion.empty()) {
            int id = cidsInRegion[rng() % (int)cidsInRegion.size()];
            int i = corners[id].first;
            int j = corners[id].second;
            { // MOV
                for (int di = -1; di <= 1; di++)
                    for (int dj = -1; dj <= 1; dj++)
                        if (di != 0 || dj != 0) {
                          int ni = i + di;
                          int nj = j + dj;
                          if (ni < r1 || nj < c1 || ni >= r2 || nj >= c2 || top[ni][nj] == make_pair(ni, nj) || (ni == 0 && nj == 0))
                            continue;
                          {
                            int L = 0;
                            int R = (int) corners.size();
                            for (int it = 0; it < (int) corners.size(); it++) {
                              if (corners[it].first <= ni && corners[it].second <= nj) {
                                L = max(L, it + 1);
                              }
                              if (ni <= corners[it].first && nj <= corners[it].second) {
                                R = min(R, it);
                              }
                            }
                            if (L > id || id > R) {
                                continue;
                            }
                          }
                          auto old_total = total;
                          RemoveCorner(i, j);
                          AddCorner(ni, nj, id);
                          if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
                            wlog("MOV");
                            setlocal
                            i = ni;
                            j = nj;
                          } else {
                            RemoveCorner(ni, nj);
                            AddCorner(i, j, id);
                          }
                      }
            }
          }

          int i, j;
          { // ADD
            do {
                i = r1 + rng() % (r2 - r1);
                j = c1 + rng() % (c2 - c1);
            } while (top[i][j] == make_pair(i, j));
            // if (localTries > 0) localTries--;
            auto old_total = total;
            AddCorner(i, j, -1);
            if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
              wlog("ADD");
              setlocal
            } else {
              RemoveCorner(i, j);
            }
          }

          if (!cidsInRegion.empty()) { // REM
            int id = cidsInRegion[rng() % (int)cidsInRegion.size()];
            i = corners[id].first;
            j = corners[id].second;
            auto old_total = total;
            RemoveCorner(i, j);
            if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
              wlog("REM");
              setlocal
            } else {
              AddCorner(i, j, id);
            }
          }
        }

        cerr << " -> " << total / 1000.0 << endl;
        /*if (start_total < total) {
            for (int i = r1; i < r2; i++)
                for (int j = c1; j < c2; j++)
                    if (top[i][j] == make_pair(i, j)) {
                        RemoveCorner(i, j);
                    }
            reverse(save.begin(), save.end());
            for (auto [c, id] : save)
                AddCorner(c.first, c.second, id);
        }*/
        return true;
    };

    auto optimizeOneByOne = [&]() {
        for (int it = 0; it < 100000000; it++) {
          if (total < best_total) {
            best_total = total;
            rects.clear();
            rects.emplace_back(make_pair(0, 0), paint_into[0][0]);
            for (auto& p : corners) {
              rects.emplace_back(p, paint_into[p.first][p.second]);
            }
          }
          if (GetTime() > optSeconds || !optRunning) {
            break;
          }

          if (hardMove) {
            hardMove = false;
            int id = rng() % (int) corners.size();
            int i = corners[id].first;
            int j = corners[id].second;
            vector<pair<pair<int, int>, int>> toMove;
            for (size_t cc = 0; cc < corners.size(); cc++)
                if (abs(corners[cc].first - i) < 42 && abs(corners[cc].second - j) < 42) {
                    toMove.emplace_back(corners[cc], cc);
                }
            if (toMove.size() > 5) toMove.resize(5);
            for (auto [p, id] : toMove) {
                auto [ii, jj] = p;
                RemoveCorner(ii, jj);
                ii += rng() % 11 - 5;
                jj += rng() % 11 - 5;
                if (ii < 0) ii = 0;
                if (ii >= N) ii = N -1;
                if (jj < 0) jj = 0;
                if (jj >= N) jj = N - 1;
                AddCorner(ii, jj, id);
            }
            cerr << "Moved " << toMove.size() << ", total: " << total << endl;
          }

          if (it % 100 == 0) {
            T = T * 0.9999;
          }
          if (corners.size() >= 2) {
            int id = rng() % (int) corners.size();
            int i = corners[id].first;
            int j = corners[id].second;
            bool bad = false;
            if (localTries > 0) {
                localTries--;
                if (abs(i - localI) > 40 || abs(j - localJ) > 40) {
                    bad = true;
                }
            }
            if (!bad) {
                auto old_total = total;
                RemoveCorner(i, j);
                AddCorner(i, j, -1);
                if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
                  wlog("SWP");
                  setlocal
                } else {
                  RemoveCorner(i, j);
                  AddCorner(i, j, id);
                }
            }
          }
          if (!corners.empty()) {
            int id = rng() % (int) corners.size();
            int i = corners[id].first;
            int j = corners[id].second;
            bool bad = false;
            if (localTries > 0) {
                localTries--;
                if (abs(i - localI) > 40 || abs(j - localJ) > 40) {
                    bad = true;
                }
            }
            if (!bad) {
                for (int di = -1; di <= 1; di++)
                    for (int dj = -1; dj <= 1; dj++)
                        if (di != 0 || dj != 0) {
                          int ni = i + di;
                          int nj = j + dj;
                          if (ni < 0 || nj < 0 || ni >= N || nj >= N || top[ni][nj] == make_pair(ni, nj) || (ni == 0 && nj == 0))
                            continue;
                          {
                            int L = 0;
                            int R = (int) corners.size();
                            for (int it = 0; it < (int) corners.size(); it++) {
                              if (corners[it].first <= ni && corners[it].second <= nj) {
                                L = max(L, it + 1);
                              }
                              if (ni <= corners[it].first && nj <= corners[it].second) {
                                R = min(R, it);
                              }
                            }
                            if (L > id || id > R) {
                                continue;
                            }
                          }
                          auto old_total = total;
                          RemoveCorner(i, j);
                          AddCorner(ni, nj, id);
                          if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
                            wlog("MOV");
                            setlocal
                            i = ni;
                            j = nj;
                          } else {
                            RemoveCorner(ni, nj);
                            AddCorner(i, j, id);
                          }
                      }
            }
          }

          int i, j, si, sj;
          { // ADD
            do {
              // qit++;
              // i = qit % (N * N) / N;
              // j = qit % N;
                i = rng() % N;
                j = rng() % N;
                si = rng() % 20 + 1;
                sj = rng() % 20 + 1;
            } while (top[i][j] == make_pair(i, j) || i + si >= N || j + sj >= N || top[i+si][j] == make_pair(i+si, j) || top[i][j+sj] == make_pair(i, j+sj) || (localTries > 0 && (abs(i - localI) > 20 || abs(j - localJ) > 20)));
            // if (localTries > 0) localTries--;
            auto old_total = total;
            AddCorner(i, j, -1);
            AddCorner(i+si, j, -1);
            AddCorner(i, j+sj, -1);
            if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
              wlog("ADD");
              setlocal
            } else {
              RemoveCorner(i, j);
              RemoveCorner(i+si, j);
              RemoveCorner(i, j+sj);
            }
          }

          { // REM
            int id = rng() % (int) corners.size();
            i = corners[id].first;
            j = corners[id].second;
            if (localTries > 0) {
                localTries--;
                if (abs(i - localI) > 40 || abs(j - localJ) > 40) {
                    continue;
                }
            }
            auto old_total = total;
            RemoveCorner(i, j);
            if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
              wlog("REM");
              setlocal
            } else {
              AddCorner(i, j, id);
            }
          }
        }
    };

    auto optimizeRegions = [&]() {
        int R = 25;
        int RS = N / R;
        assert(N % R == 0);
        vector<vector<double>> regionsWeight(R, vector<double>(R, 1));
        for (int it = 0; it < 100000000; it++) {
          if (total < best_total) {
            best_total = total;
            rects.clear();
            rects.emplace_back(make_pair(0, 0), paint_into[0][0]);
            for (auto& p : corners) {
              rects.emplace_back(p, paint_into[p.first][p.second]);
            }
          }
          if (GetTime() > optSeconds || !optRunning) {
            break;
          }

          int ri = 0, rj = 0;
          vector<int> cidsInRegion;
          // double v = GetTime();
          while (true) {
              double tw = 0;
              for (int i = 0; i < R; i++)
                for (int j = 0; j < R; j++)
                  tw += regionsWeight[i][j];

              double coin = urd(rng) * tw;
              ri = R - 1, rj = R - 1;
              for (int i = 0; i < R; i++)
                for (int j = 0; j < R; j++) {
                  tw -= regionsWeight[i][j];
                  if (tw < coin) {
                    ri = i;
                    rj = j;
                    goto out;
                  }
                }
              out:;

            for (size_t id = 0; id < corners.size(); id++)
                if (corners[id].first / RS == ri && corners[id].second / RS == rj) {
                    cidsInRegion.push_back(id);
                }

            if (!cidsInRegion.empty()) break;
          }
          // cerr << "passed " << GetTime() - v << "s\n";

          bool impr = false;
          int it_start_total = total;

          if (!corners.empty()) {
            for (int id : cidsInRegion) {
                int i = corners[id].first;
                int j = corners[id].second;
                int conts = 0;
                while (true) {
                  int ni = i - 3 + rng() % 7;
                  int nj = j - 3 + rng() % 7;
                  if (ni < 0 || nj < 0 || ni >= N || nj >= N || top[ni][nj] == make_pair(ni, nj) || (ni == 0 && nj == 0)) {
                    if (++conts > 5) {
                      break;
                    }
                    continue;
                  }
                  {
                    int L = 0;
                    int R = (int) corners.size();
                    for (int it = 0; it < (int) corners.size(); it++) {
                      if (corners[it].first <= ni && corners[it].second <= nj) {
                        L = max(L, it + 1);
                      }
                      if (ni <= corners[it].first && nj <= corners[it].second) {
                        R = min(R, it);
                      }
                    }
                    if (L > id || id > R) {
                      if (++conts > 5) {
                        break;
                      }
                      continue;
                    }
                  }
                  conts = 0;
                  auto old_total = total;
                  RemoveCorner(i, j);
                  AddCorner(ni, nj, id);
                  if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
                    wlog("MOV");
                    impr |= total < it_start_total;
                    i = ni;
                    j = nj;
                  } else {
                    RemoveCorner(ni, nj);
                    AddCorner(i, j, id);
                    break;
                  }
                }
            }
          }
          // cerr << "passed " << GetTime() - v << "s\n";

          if (it % 3 == 0) { // SWP
              if (corners.size() >= 2) {
                for (int id : cidsInRegion) {
                    int i = corners[id].first;
                    int j = corners[id].second;
                    auto old_total = total;
                    RemoveCorner(i, j);
                    AddCorner(i, j, -1);
                    if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
                      wlog("SWP");
                      impr |= total < it_start_total;
                      break;
                    } else {
                      RemoveCorner(i, j);
                      AddCorner(i, j, id);
                    }
                }
              }
              // cerr << it % 3 << " op, passed " << GetTime() - v << "s\n";
          } else if (it % 3 == 1) { // ADD
            for (int i = ri * RS; i < (ri + 1) * RS; i++)
                for (int j = rj * RS; j < (rj + 1) * RS; j++) {
                    if (top[i][j] == make_pair(i, j)) continue;
                    if (rng() % 17) continue;

                    auto old_total = total;
                    AddCorner(i, j, -1);
                    if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
                      wlog("ADD");
                      impr |= total < it_start_total;
                    } else {
                      RemoveCorner(i, j);
                    }
                }
            // cerr << it % 3 << " op, passed " << GetTime() - v << "s\n";
          } else if (it > 123) { // REM
            for (int id : cidsInRegion) {
                int i = corners[id].first;
                int j = corners[id].second;
                auto old_total = total;
                RemoveCorner(i, j);
                if (total <= old_total || exp((old_total - total) / 10000.0 / T) > urd(rng)) {
                  wlog("REM");
                  impr |= total < it_start_total;
                  break;
                } else {
                  AddCorner(i, j, id);
                }
            }
            // cerr << it % 3 << " op, passed " << GetTime() - v << "s\n";
          }

          const double lambda = 0.8;
          const double goodWeight = 50;
          if (impr) {
            regionsWeight[ri][rj] = goodWeight * lambda + (1 - lambda) * regionsWeight[ri][rj];
            for (int di = -1; di <= 1; di++)
                for (int dj = -1; dj <= 1; dj++) {
                    int nri = ri + di;
                    int nrj = rj + dj;
                    if (nri >= 0 && nri < R && nrj >= 0 && nrj < R)
                        regionsWeight[nri][nrj] = goodWeight * lambda * lambda + (1 - lambda * lambda) * regionsWeight[ri][rj];
                }
          } else regionsWeight[ri][rj] = 1 * lambda + (1 - lambda) * regionsWeight[ri][rj];
          // cerr << "end, passed " << GetTime() - v << "s\n";
          // msg << "[" << ri << ", " << rj << "] corners in region: " << cidsInRegion.size();
        }
    };

    if (hardRects) {
        while (true) {
            if (GetTime() > optSeconds || !optRunning) {
                break;
            }

            int r1, c1, r2, c2;
            while (true) {
                r1 = rng() % N;
                c1 = rng() % N;
                r2 = rng() % N;
                c2 = rng() % N;
                if (c1 > c2) swap(c1, c2);
                if (r1 > r2) swap(r1, r2);
                if (r1 == r2 || c1 == c2) continue;
                if (r2 - r1 < 40 || c2 - c1 < 40) continue;
                if ((r2 - r1) * (c2 - c1) > 12345) continue;
                if ((r2 - r1) * (c2 - c1) < 2555) continue;

                break;
            }

            if (optimizeHard(r1, c1, r2, c2, hardIters))
                break;
        }
    } else if (regionOpt)
        optimizeRegions();
    else
        optimizeOneByOne();
    drawR2 = drawC2 = 0;
/*    sort(rects.begin(), rects.end(), [&](auto& r1, auto& r2) {
      return Priority(r1.first) < Priority(r2.first);
    });*/
    auto Compare = [&](int x, int y) {
      int cand1 = llround(costs.merge * (n * m) / (max(x, n - x) * y));
      cand1    += llround(costs.merge * (n * m) / (max(x, n - x) * (m - y)));
      cand1    += llround(costs.merge * m / max(y, m - y));
      int cand2 = llround(costs.merge * (n * m) / (x * max(y, m - y)));
      cand2    += llround(costs.merge * (n * m) / ((n - x) * max(y, m - y)));
      cand2    += llround(costs.merge * n / max(x, m - x));
      return cand1 < cand2;
    };
    for (int it = 0; it < (int) rects.size(); it++) {
      int i = it;
      int xa = rects[i].first.first;
      int ya = rects[i].first.second;
//      cerr << "xa " << xa << " " << ya << endl;
      Color paint_into = rects[i].second;
      if (xa == 0 && ya == 0) {
        res.ins.push_back(ColorIns(to_string(idx), paint_into));
      }
      if (xa == 0 && ya > 0) {
        res.ins.push_back(SplitYIns(to_string(idx), ya));
        res.ins.push_back(ColorIns(to_string(idx) + ".1", paint_into));
        res.ins.push_back(MergeIns(to_string(idx) + ".0", to_string(idx) + ".1"));
        idx += 1;
      }
      if (xa > 0 && ya == 0) {
        res.ins.push_back(SplitXIns(to_string(idx), xa));
        res.ins.push_back(ColorIns(to_string(idx) + ".1", paint_into));
        res.ins.push_back(MergeIns(to_string(idx) + ".0", to_string(idx) + ".1"));
        idx += 1;
      }
      if (xa > 0 && ya > 0) {
        res.ins.push_back(SplitPointIns(to_string(idx), xa, ya));
        res.ins.push_back(ColorIns(to_string(idx) + ".2", paint_into));
        if (Compare(n - xa, n - ya)) {
          res.ins.push_back(MergeIns(to_string(idx) + ".3", to_string(idx) + ".2"));
          res.ins.push_back(MergeIns(to_string(idx) + ".0", to_string(idx) + ".1"));
        } else {
          res.ins.push_back(MergeIns(to_string(idx) + ".3", to_string(idx) + ".0"));
          res.ins.push_back(MergeIns(to_string(idx) + ".2", to_string(idx) + ".1"));
        }
        res.ins.push_back(MergeIns(to_string(idx + 1), to_string(idx + 2)));
        idx += 3;
      }
    }

    for (int rep = 0; rep < mode; rep++) {
      res.rotateClockwise();
    }

    res.ins.insert(res.ins.begin(), res_pref.ins.begin(), res_pref.ins.end());

    res.score += round(best_total * 0.001);
    msg.clear() << "Duration: " << GetTime() << "s\n";
    postprocess(res);
}

void solveOptCycle() {
    while (optRunning) {
        solveOpt();
    }
}

double distColor(const Color& a, const Color& b) {
    return sqrt(
        sqr(a[0] - b[0]) +
        sqr(a[1] - b[1]) +
        sqr(a[2] - b[2]) +
        sqr(a[3] - b[3])
    );
}

void GetRekt() {
    set<pair<int, int>> used;
    for (const auto& b : coloredBlocks) {
        if (b.r2 != N || b.c2 != N) {
            msg << "Sorry, only topright\n";
            return;
        }
        used.insert(make_pair(b.r1, b.c1));
    }
    /*for (int i = 2; i < N; i++)
        for (int j = 2; j < N; j++)
            if (used.find(make_pair(i, j)) == used.end()) {
                double diffI = (distColor(colors[i][j], colors[i-1][j]) +
                               distColor(colors[i][j], colors[i-2][j])) / 2;
                double diffJ = (distColor(colors[i][j], colors[i][j-1]) +
                               distColor(colors[i][j], colors[i][j-2])) / 2;
                if (diffI > 150 && diffJ > 150) {
                    coloredBlocks.push_back(Block{i, j, N, N, colors[i][j]});
                    int idx = coloredBlocks.size() - 1;
                    while (idx > 0 && (coloredBlocks[idx].r1 < coloredBlocks[idx - 1].r1 || coloredBlocks[idx].c1 < coloredBlocks[idx - 1].c1)) {
                        swap(coloredBlocks[idx], coloredBlocks[idx - 1]);
                        idx--;
                    }
                }
            }*/
    for (int i = RS; i < N; i += RS)
        for (int j = RS; j < N; j += RS)
            if (used.find(make_pair(i, j)) == used.end()) {
                Color sum;
                for (int q = 0; q < 4; q++) sum[q] = 0;
                int total = 0;
                for (int di = 0; di < RS; di++)
                    for (int dj = 0; dj < RS; dj++)
                        if (i + di < N && j + dj < N) {
                            total++;
                            for (int q = 0; q < 4; q++)
                                sum[q] += colors[i+di][j+dj][q];
                        }
                Color c;
                for (int q = 0; q < 4; q++) c[q] = sum[q] / total;

                coloredBlocks.push_back(Block{i, j, N, N, c});
                int idx = coloredBlocks.size() - 1;
                while (idx > 0 && (coloredBlocks[idx].r1 < coloredBlocks[idx - 1].r1 || coloredBlocks[idx].c1 < coloredBlocks[idx - 1].c1)) {
                    swap(coloredBlocks[idx], coloredBlocks[idx - 1]);
                    idx--;
                }
            }
}

void swapRects(int r1, int c1, int r2, int c2, int sr, int sc) {
    for (int i = 0; i < sr; i++)
        for (int j = 0; j < sc; j++)
            swap(colors[r1+i][c1+j], colors[r2+i][c2+j]);

    for (auto& b : coloredBlocks) {
        if (r1 <= b.r1 && b.r1 < r1 + sr && c1 <= b.c1 && b.c1 < c1 + sc) {
            b.r1 = b.r1 - r1 + r2;
            b.c1 = b.c1 - c1 + c2;
        }
        if (r2 <= b.r1 && b.r1 < r2 + sr && c2 <= b.c1 && b.c1 < c2 + sc) {
            b.r1 = b.r1 - r2 + r1;
            b.c1 = b.c1 - c2 + c1;
        }
    }

    for (size_t j = 1; j < coloredBlocks.size(); j++) {
        int idx = j;
        while (idx > 0 && (coloredBlocks[idx].r1 < coloredBlocks[idx - 1].r1 || coloredBlocks[idx].c1 < coloredBlocks[idx - 1].c1)) {
            swap(coloredBlocks[idx], coloredBlocks[idx - 1]);
            idx--;
        }
    }
}