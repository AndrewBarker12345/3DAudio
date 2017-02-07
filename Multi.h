/*
 Multi.h
 
 Holds multiple, uniquely-IDed things (like a std::map, but underlying container is a vector).

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

#ifndef Multi_h
#define Multi_h

#include <vector>

template <class Thing>
class Multi
{
private:
    class IDedThing
    {
    public:
        template <class... Args>
        IDedThing(const std::size_t idNumber, Args&&... args) : thing(std::forward<Args>(args)...), idNum(idNumber) {}
        Thing thing;
        std::size_t idNum = 0;
    };
    std::vector<IDedThing> things;
    
public:
    Thing* get(const std::size_t idNumber) noexcept
    {
        auto index = std::find_if(std::begin(things), std::end(things),
                                  [&](const auto& thing){return thing.idNum == idNumber;})
                     - std::begin(things);
        if (index < things.size())
            return &things[index].thing;
        else
            return nullptr;
    }
    
    const Thing* get(const std::size_t idNumber) const noexcept
    {
        return get(idNumber);
    }
    
    std::vector<IDedThing>& getVector() noexcept
    {
        return things;
    }
    
    const std::vector<IDedThing>& getVector() const noexcept
    {
        return things;
    }
    
    template <class... Args>
    void add(const std::size_t idNumber, Args&&... args)
    {
        remove(idNumber);
        things.emplace_back(idNumber, std::forward<Args>(args)...);
    }
    
    void remove(const std::size_t idNumber)
    {
        things.erase(std::remove_if(std::begin(things), std::end(things),
                                    [&](const auto& thing){return thing.idNum == idNumber;}),
                     std::end(things));
    }
    
    void clear() noexcept
    {
        things.clear();
    }
    
    std::size_t size()
    {
        return things.size();
    }
};

#endif /* Multi_h */
