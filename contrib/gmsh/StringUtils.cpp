// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#if defined(__CYGWIN__)
#include <sys/cygwin.h>
#endif

#include "StringUtils.h"
#include "GmshMessage.h"

void SwapBytes(char *array, int size, int n)
{
  char *x = new char[size];
  for(int i = 0; i < n; i++) {
    char *a = &array[i * size];
    memcpy(x, a, size);
    for(int c = 0; c < size; c++)
      a[size - 1 - c] = x[c];
  }
  delete [] x;
}

std::string ExtractDoubleQuotedString(const char *str, int len)
{
  char *c = strstr((char*)str, "\"");
  if(!c) return "";
  std::string ret;
  for(int i = 1; i < len; i++) {
    if(c[i] == '"' || c[i] == EOF || c[i] == '\n' || c[i] == '\r') break;
    ret.push_back(c[i]);
  }
  return ret;
}

std::string SanitizeTeXString(const char *in, int equation)
{
  // if there is a '$' or a '\' in the string, assume the author knows
  // what he's doing:
  if(strstr(in, "$") || strstr(in, "\\"))
    return std::string(in);

  std::string out;

  if(equation) out.push_back('$');

  // otherwise, escape the following special characters:
  char bad[8] = { '%', '^', '#', '%', '&', '_', '{', '}' };
  while(*in){
    for(unsigned int i = 0; i < sizeof(bad); i++){
      if(*in == bad[i]){
        out.push_back('\\');
        break;
      }
    }
    out.push_back(*in++);
  }

  if(equation) out.push_back('$');
  return out;
}

std::string FixWindowsPath(const char *in)
{
#if defined(__CYGWIN__)
  char tmp[1024];
  cygwin_conv_to_win32_path(in, tmp);
  return std::string(tmp);
#else
  return std::string(in);
#endif
}

std::vector<std::string> SplitFileName(std::string fileName)
{
  // returns [path, baseName, extension]
  int idot = fileName.find_last_of('.');
  int islash = fileName.find_last_of("/\\");
  if(idot == std::string::npos) idot = -1;
  if(islash == std::string::npos) islash = -1;
  std::vector<std::string> s(3);
  if(idot > 0)
    s[2] = fileName.substr(idot);
  if(islash > 0)
    s[0] = fileName.substr(0, islash + 1);
  s[1] = fileName.substr(s[0].size(), fileName.size() - s[0].size() - s[2].size());
  return s;
}

std::vector<std::string> SplitWhiteSpace(std::string in, unsigned int len)
{
  std::vector<std::string> out(1);
  for(unsigned int i = 0; i < in.size(); i++){
    out.back() += in[i];
    if(out.back().size() > len && in[i] == ' ')
      out.resize(out.size() + 1);
  }
  return out;
}

void ReplaceMultiFormat(const char *in, const char *val, char *out)
{
  unsigned int i = 0, j = 0;

  out[0] = '\0';
  while(i < strlen(in)){
    if(in[i] == '%' && i != strlen(in) - 1){
      if(in[i + 1] == 's'){
        strcat(out, val);
        i += 2;
        j += strlen(val);
      }
      else{
        Msg::Warning("Skipping unknown format '%%%c' in '%s'", in[i + 1], in);
        i += 2;
      }
    }
    else{
      out[j] = in[i];
      out[j + 1] = '\0';
      i++;
      j++;
    }
  }
  out[j] = '\0';
}
