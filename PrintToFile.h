//
//  PrintToFile.h
//  ThreeDAudio
//
//  Created by Andrew Barker on 9/7/16.
//
//

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

//static void debug(const std::string& text,
//                  const PrintToFileMode mode = PrintToFileMode::OVERWRITE, // append if false
//                  const std::string& fileName = "/Users/AndrewBarker/Desktop/debug.txt")
//{
//    std::stringstream tmp;
//    if (mode == PrintToFileMode::APPEND) {
//        std::ofstream f (fileName, std::ios::in);
//        tmp << f.rdbuf();
//    }
//    std::ofstream f (fileName);
//    if (mode == PrintToFileMode::APPEND)
//        f << tmp.rdbuf();
//    f << text << std::endl;
//}

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

//template <class... Args>
//static void debug(const Args&... args,
//                  const std::string& fileName = "Debugger.txt",
//                  const bool overwrite = true) // append if false
//{
//    std::ofstream f (fileName);
//    //debug<int, Args...>(args..., f);
//    f.close();
//}
//
////template <class Arg, class... Args>
////static void debug(const Arg& arg,
////                  const Args&... args,
////                  const std::ofstream& f)
////{
////    f << std::to_string(arg) << std::endl;
////    debug(args..., f);
////}
//
//template <class Arg>
//static void bobfunc(std::ofstream& f, Arg& arg)
//{
//    f << std::to_string(arg) << std::endl;
//}
//
//template <class Arg, class... Args>
//static void bobfunc(std::ofstream& f, Arg& arg, Args&... args)
//{
//    bobfunc(f, arg, args...);
//}
//
//template <class Arg, class... Args>
//static void bobfunc(Arg& arg, Args&... args, const std::string& fileName = "Debugger.txt")
//{
//    std::ofstream f (fileName);
//    //arg = -1;
//    bobfunc(f, arg, args...);
//    //bobfunc(args...);
//    f.close();
//}


#endif /* PrintToFile_h */
