/*
 PrintToFile.h
 
 For printing stuff to a file to debug.

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

#ifndef PrintToFile_h
#define PrintToFile_h

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#define NAMEANDVALUE(var)  std::string( #var ) + ":  " + std::to_string( (var) )
#define NAMEANDVALUESTR(var)  std::string( #var ) + ":  " + var
enum class PrintToFileMode { OVERWRITE, APPEND };
// ex:  debug({"bob", std::to_string(3.5f), ...});

static void debug(const std::vector<std::string>& textByLine,
                  const PrintToFileMode mode = PrintToFileMode::OVERWRITE, // append if false
                  const std::string& fileName = "/Users/AndrewBarker/Desktop/debug.txt")
{
    std::stringstream tmp;
    if (mode == PrintToFileMode::APPEND) {
        std::ofstream f (fileName, std::ios::in);
        tmp << f.rdbuf();
        //f.close();
    }
    std::ofstream f (fileName);
    if (mode == PrintToFileMode::APPEND)
        f << tmp.rdbuf();
    for (const auto& l : textByLine)
        f << l << std::endl;
    //f.close();
}

//// single line of text version
//static void debug(const std::string& text,
//                  const PrintToFileMode mode = PrintToFileMode::OVERWRITE, // append if false
//                  const std::string& fileName = "/Users/AndrewBarker/Desktop/debug.txt")
//{
//    debug({text}, mode, fileName);
//}

#endif /* PrintToFile_h */
