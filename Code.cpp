#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>

using namespace std;

const int ZONES = 30, ALPHABET = 27, MAX_TIME = 1800;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

struct Solution { 
    vector<int> assign; 
    int start, score; 
    
    Solution() : start(0), score(0) {}
    Solution(int n) : assign(n), start(0), score(0) {}
};

Solution genInit(const vector<int>& targets) {
    Solution sol(targets.size());
    sol.start = uniform_int_distribution<int>(0, ZONES-1)(rng);
    vector<int> runes(ZONES, 0);
    int pos = sol.start;
    
    for (size_t i = 0; i < targets.size(); i++) {
        if (uniform_real_distribution<double>(0,1)(rng) < 0.7 && i) {
            int bestZ = -1, bestC = 1000000000;
            for (int z = 0; z < ZONES; z++) {
                int move = min(abs(z-pos), ZONES-abs(z-pos));
                int cur = runes[z], tar = targets[i];
                int change = min((tar-cur+ALPHABET)%ALPHABET, (cur-tar+ALPHABET)%ALPHABET);
                if (cur == tar) change = 0;
                if (move+change < bestC) {
                    bestC = move+change;
                    bestZ = z;
                }
            }
            sol.assign[i] = bestZ;
            pos = bestZ;
        } else {
            sol.assign[i] = uniform_int_distribution<int>(0, ZONES-1)(rng);
            pos = sol.assign[i];
        }
        runes[pos] = targets[i];
    }
    return sol;
}

int evaluate(const Solution& s, const vector<int>& targets) {
    vector<int> runes(ZONES, 0);
    int pos = s.start, cost = 0;
    for (size_t i = 0; i < targets.size(); i++) {
        int z = s.assign[i];
        cost += min(abs(z-pos), ZONES-abs(z-pos));
        int cur = runes[z], tar = targets[i];
        cost += min((tar-cur+ALPHABET)%ALPHABET, (cur-tar+ALPHABET)%ALPHABET) + 1;
        runes[z] = tar;
        pos = z;
    }
    return cost;
}

void localImprove(Solution& s, const vector<int>& targets) {
    vector<int> runes(ZONES, 0);
    int pos = s.start;
    for (size_t i = 0; i < targets.size(); i++) {
        int bestZ = s.assign[i], bestC = 1000000000;
        for (int z = 0; z < ZONES; z++) {
            int move = min(abs(z-pos), ZONES-abs(z-pos));
            int cur = runes[z], tar = targets[i];
            int change = cur==tar ? 0 : min((tar-cur+ALPHABET)%ALPHABET, (cur-tar+ALPHABET)%ALPHABET);
            if (move+change < bestC) {
                bestC = move+change;
                bestZ = z;
            }
        }
        s.assign[i] = bestZ;
        pos = bestZ;
        runes[pos] = targets[i];
    }
    s.score = evaluate(s, targets);
}

Solution annealing(const vector<int>& targets, chrono::steady_clock::time_point startTime) {
    Solution cur = genInit(targets);
    cur.score = evaluate(cur, targets);
    localImprove(cur, targets);
    Solution best = cur;
    
    double temp = min(50.0 + targets.size()*0.1, 200.0), cool = 0.999;
    uniform_real_distribution<double> prob(0,1);
    uniform_int_distribution<int> idx(0, targets.size()-1), zone(0, ZONES-1);
    
    int noImp = 0;
    while (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now()-startTime).count() < MAX_TIME) {
        Solution neigh = cur;
        
        double mut = prob(rng);
        if (mut < 0.4) {
            neigh.assign[idx(rng)] = zone(rng);
        } else if (mut < 0.7) {
            int numChanges = max(1, (int)targets.size()/20);
            for (int k = 0; k < numChanges; k++) {
                neigh.assign[idx(rng)] = zone(rng);
            }
        } else if (mut < 0.85 && targets.size() > 1) {
            int i = idx(rng);
            int j = idx(rng);
            if (i != j) {
                swap(neigh.assign[i], neigh.assign[j]);
            }
        } else if (mut < 0.95) {
            neigh.start = zone(rng);
        } else if (targets.size() > 10) {
            int st = uniform_int_distribution<int>(0, targets.size()-10)(rng);
            int end = min(st + uniform_int_distribution<int>(5,15)(rng), (int)targets.size()-1);
            for (int i = st; i <= end; i++) {
                neigh.assign[i] = zone(rng);
            }
        }
        
        neigh.score = evaluate(neigh, targets);
        if (prob(rng) < 0.1) {
            localImprove(neigh, targets);
        }
        
        int delta = neigh.score - cur.score;
        if (delta < 0 || prob(rng) < exp(-delta/temp)) {
            cur = neigh;
            if (cur.score < best.score) {
                best = cur;
                noImp = 0;
            } else {
                noImp++;
            }
        } else {
            noImp++;
        }
        
        temp *= cool;
        if (noImp > 100) {
            temp *= 1.1;
            noImp = 0;
        }
    }
    return best;
}

string toCommands(const Solution& s, const vector<int>& targets) {
    vector<int> runes(ZONES, 0);
    string res;
    int pos = s.start;
    
    for (size_t i = 0; i < targets.size(); i++) {
        int z = s.assign[i];
        int d = abs(z-pos);
        int w = ZONES-d;
        
        if (d <= w) {
            if (z > pos) {
                res += string(d, '>');
            } else {
                res += string(d, '<');
            }
        } else {
            if (z > pos) {
                res += string(w, '<');
            } else {
                res += string(w, '>');
            }
        }
        
        pos = z;
        
        int cur = runes[z];
        int tar = targets[i];
        if (cur != tar) {
            int fwd = (tar-cur+ALPHABET)%ALPHABET;
            int bwd = (cur-tar+ALPHABET)%ALPHABET;
            if (fwd <= bwd) {
                res += string(fwd, '+');
            } else {
                res += string(bwd, '-');
            }
            runes[z] = tar;
        }
        
        res += '.';
    }
    
    if (res.length() > 4000) {
        res = res.substr(0, 4000);
    }
    return res;
}

int main() {
    auto startTime = chrono::steady_clock::now();
    string phrase; 
    getline(cin, phrase);
    
    vector<int> targets;
    for (char c : phrase) {
        targets.push_back(c == ' ' ? 0 : c - 'A' + 1);
    }
    
    Solution best = annealing(targets, startTime);
    
    if (targets.size() < 50) {
        for (int i = 0; i < 5; i++) {
            auto currentTime = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<chrono::milliseconds>(currentTime - startTime).count();
            if (elapsed > MAX_TIME * 0.9) {
                break;
            }
            
            Solution cand = annealing(targets, currentTime);  // Исправить тут
            if (cand.score < best.score) {
                best = cand;
            }
        }
    }
    
    localImprove(best, targets);
    cout << toCommands(best, targets) << endl;
    
    return 0;
}
