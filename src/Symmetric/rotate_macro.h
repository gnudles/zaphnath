
//clang queries
#ifndef __has_feature         // Optional of course.
  #define __has_feature(x) 0  // Compatibility with non-clang compilers.
#endif
#ifndef __has_extension
  #define __has_extension __has_feature // Compatibility with pre-3.0 compilers.
#endif
#ifndef __has_builtin    
#define __has_builtin(x) 0  // Compatibility with non-clang compilers. 
#endif 




#if __has_builtin(__builtin_rotateleft16)
#define ROTL16(Y,B) (__builtin_rotateleft16(Y,B))
#else
#define ROTL16(Y,B) (((Y)<<(B))|((Y)>>((-(B))&0xf)))
#endif

#if __has_builtin(__builtin_rotateleft32)
#define ROTL32(Y,B) (__builtin_rotateleft32(Y,B))
#else
#define ROTL32(Y,B) (((Y)<<(B))|((Y)>>((-(B))&0x1f)))
#endif

#if __has_builtin(__builtin_rotateleft64)
#define ROTL64(Y,B) (__builtin_rotateleft64(Y,B))
#else
#define ROTL64(Y,B) (((Y)<<(B))|((Y)>>((-(B))&0x3f)))
#endif



#if __has_builtin(__builtin_rotateright16)
#define ROTR16(Y,B) (__builtin_rotateright16(Y,B))
#else
#define ROTR16(Y,B) (((Y)>>(B))|((Y)<<((-(B))&0xf)))
#endif

#if __has_builtin(__builtin_rotateright32)
#define ROTR32(Y,B) (__builtin_rotateright32(Y,B))
#else
#define ROTR32(Y,B) (((Y)>>(B))|((Y)<<((-(B))&0x1f)))
#endif

#if __has_builtin(__builtin_rotateright64)
#define ROTR64(Y,B) (__builtin_rotateright64(Y,B))
#else
#define ROTR64(Y,B) (((Y)>>(B))|((Y)<<((-(B))&0x3f)))
#endif





#define ROTL128(Y,B)\
{\
uint64_t temp;\
if(B < 64){temp=Y[0];Y[0]=(Y[0]<<(B))|(Y[1]>>(64-B));Y[1]=(Y[1]<<(B))|(temp>>(64-B));}\
else{temp=Y[0];Y[0]=(Y[1]<<(B-64))|(Y[0]>>(128-B));Y[1]=(temp<<(B-64))|(Y[1]>>(128-B));}\
}

