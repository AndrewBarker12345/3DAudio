//
//  Multi.h
//
//  Created by Andrew Barker on 4/15/16.
//
//

#ifndef Multi_h
#define Multi_h

#include <vector>

// holds multiple, uniquely-IDed things (like a std::map, but underlying container is a vector)
template <class Thing>
class Multi
{
private:
    // can't use POD's with inheritance
//    class IDedThing : public Thing
//    {
//    public:
//        template <class... Args>
//        IDedThing(std::size_t idNumber, Args&&... args) : Thing(std::forward<Args>(args)...), idNum(idNumber) {}
//        Thing thing;
//        std::size_t idNum = 0;
//    };
    class IDedThing
    {
    public:
        //IDedThing(std::size_t idNumber, Thing thing) : thing(thing), idNum(idNumber) {}
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
//        std::vector<Thing> justThings;
//        justThings.reserve(things.size());
//        for (const auto& thing : things)
//            justThings.emplace_back(thing);
//        return justThings;
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
