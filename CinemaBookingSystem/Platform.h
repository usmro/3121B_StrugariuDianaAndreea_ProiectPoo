#ifndef PLATFORM_H
#define PLATFORM_H

#include <string>
#include <iostream>
#include <sstream>
#ifdef _WIN32
  #include <conio.h>
  #define CLEAR "cls"
  enum Tasta { T_SUS=1000,T_JOS,T_STANGA,T_DREAPTA,T_ENTER,T_ESC,T_BACKSPACE,T_ALTUL };
  inline Tasta citesteInput(int& ch) {
      int c = _getch();
      if (c==224||c==0) {
          int c2=_getch();
          if(c2==72) return T_SUS;
          if(c2==80) return T_JOS;
          if(c2==75) return T_STANGA;
          if(c2==77) return T_DREAPTA;
          return T_ALTUL;
      }
      if(c==13) return T_ENTER;
      if(c==27) return T_ESC;
      if(c==8)  return T_BACKSPACE;
      ch=c; return T_ALTUL;
  }
#else
  #include <termios.h>
  #include <unistd.h>
  #define CLEAR "clear"
  enum Tasta { T_SUS=1000,T_JOS,T_STANGA,T_DREAPTA,T_ENTER,T_ESC,T_BACKSPACE,T_ALTUL };

  inline int rawGetchar(bool blocking=true) {
      struct termios oldt,newt;
      tcgetattr(STDIN_FILENO,&oldt);
      newt=oldt;
      newt.c_lflag &= ~(ICANON|ECHO);
      newt.c_cc[VMIN]  = blocking?1:0;
      newt.c_cc[VTIME] = blocking?0:1;
      tcsetattr(STDIN_FILENO,TCSANOW,&newt);
      int c=getchar();
      tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
      return c;
  }

  inline Tasta citesteInput(int& ch) {
      int c=rawGetchar(true);
      if(c==27) {
          int c2=rawGetchar(false);
          if(c2==EOF||c2==-1) return T_ESC;
          if(c2==91) {
              int c3=rawGetchar(true);
              if(c3==65) return T_SUS;
              if(c3==66) return T_JOS;
              if(c3==67) return T_DREAPTA;
              if(c3==68) return T_STANGA;
          }
          return T_ESC;
      }
      if(c==10||c==13) return T_ENTER;
      if(c==127||c==8) return T_BACKSPACE;
      ch=c; return T_ALTUL;
  }
#endif


inline std::string citesteString(const std::string& prompt) {
    std::cout << prompt;
#ifndef _WIN32
    struct termios oldt;
    tcgetattr(STDIN_FILENO,&oldt);
    struct termios newt=oldt;
    newt.c_lflag |= (ICANON|ECHO);
    tcsetattr(STDIN_FILENO,TCSANOW,&newt);
#endif
    std::string s;
    std::getline(std::cin,s);
    if(!s.empty()&&s.back()=='\r') s.pop_back();
#ifndef _WIN32
    tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
#endif
    return s;
}


inline int citesteInt(const std::string& prompt, int minV, int maxV) {
    while(true) {
        std::string s = citesteString(prompt);
        try {
            int v = std::stoi(s);
            if(v>=minV && v<=maxV) return v;
            std::cout << "  [!] Introdu un numar intre " << minV << " si " << maxV << ".\n";
        } catch(...) {
            std::cout << "  [!] Numar invalid.\n";
        }
    }
}

#endif
