//
//  History.h
//
//  Created by Andrew Barker on 8/25/16.
//
//

#ifndef History_h
#define History_h

#include <vector>
#include <algorithm>

// this class keeps track of a history of undo/redoable states.  it also has timer so that you can group smaller transactions into a single one based on the timeout length
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
