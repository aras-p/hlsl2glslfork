#ifndef __SOURCELOC_H
#define __SOURCELOC_H

typedef struct 
{
    const char* file;
    int line;
} TSourceLoc;

#ifdef __cplusplus
extern "C" {
#endif

extern const TSourceLoc gNullSourceLoc;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <sstream>


template<typename StreamType>
StreamType& operator<<(StreamType& s, const TSourceLoc& l)
{
	if(l.file)
	{
	    if(l.file[0] == '"')
	    {
	        char stripped[256];
	        strcpy(stripped, l.file + 1);
	        stripped[strlen(stripped) - 1] = '\0';
	        s << stripped;
	    }
	    else
	    {
	        s << l.file;
	    }
	
	    // Visual Studio will jump to location if you format like this
	    // file.hlsl(22): error string
	    s << '(';
	}
    
    if(l.line > 0)
    {
        s << l.line;
    }
    else
    {
        s << '?';
    }
    
	if(l.file)
    {
	    s << ')';
    }
    
    return(s);
} 

inline void OutputLineDirective(std::stringstream& s, const TSourceLoc& l)
{
	s << "#line " << l.line;
	
	if(l.file)
	{
	    s << l.file; // Either already quoted or no quotes needed
	}
	
	s << '\n';
}

#endif

#endif /* __SOURCELOC_H */ 
