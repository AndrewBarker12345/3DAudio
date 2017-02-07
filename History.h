/*
 History.h
 
 Keeps track of a history of undo/redoable states.  Also has timer so that you can group smaller transactions into a single one based on the timeout length.

 Copyright (C) 2017  Andrew Barker
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 The author can be contacted via email at andrew.barker.12345@gmail.com.
*/

#ifndef History_h
#define History_h

#include <vector>
#include <algorithm>

template<typename State>
class History {
    
private:
    std::vector<State> history;
    int current = 0;
    float undoTimer = 0; // in sec
    float timeoutLength = 3; // in sec
    
public:
    void pushBack(const State& state)
    {
        int i = 1;
        if (history.size() > 0 && history[current] == state)
            --i; // overwrite the current history with the state passed in
        if (current + i < history.size())
            history.erase(history.begin() + current + i, history.end());
        history.emplace_back(state);
        current = history.size() - 1;
    }
    
    const State* const undo() noexcept
    {
        if (history.size() > 0) {
            current = std::max(0, current - 1);
            return &history[current];
        } else
            return nullptr;
    }
    
    const State* const redo() noexcept
    {
        if (history.size() > 0) {
            current = std::min((int)history.size() - 1, current + 1);
            return &history[current];
        } else
            return nullptr;
    }
    
    void clear()
    {
        current = 0;
        history.clear();
    }
    
    const State* const getCurrent() const noexcept
    {
        if (history.size() > 0)
            return &history[current];
        else
            return nullptr;
    }
    
    void resetTimer(const float newTimeoutLength = -1) noexcept
    {
        undoTimer = 0;
        if (newTimeoutLength > 0)
            timeoutLength = newTimeoutLength;
    }
    
    void advanceTimer(const float timeToAdvance) noexcept
    {
        if (! timerExpired())
            undoTimer += timeToAdvance;
    }
    
    bool timerExpired() const noexcept
    {
        return undoTimer >= timeoutLength;
    }
    
    int getSize() const noexcept
    {
        return history.size();
    }
};

#endif /* History_h */
