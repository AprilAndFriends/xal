/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_NORMALIZE_ENDIAN

#ifdef __BIG_ENDIAN__
        // ppc & friends need convert from littleendian to their bigendian
        #define XAL_NORMALIZE_ENDIAN(variable) \
                /*printf("normalizing " #variable "(%d) - %d\n", sizeof(variable),
 variable);*/ \
                variable=(sizeof(variable)==1 ? \
                        (variable) : \
                        sizeof(variable)==2 ? \
                         ((variable & 0xFF) << 8) | \
                         ((variable & 0xFF00) >> 8) : \
                        sizeof(variable)==4 ? \
                         ((variable & 0xFF) << 24) | \
                         ((variable & 0xFF00) << 8) | \
                         ((variable & 0xFF0000) >> 8) | \
                         ((variable & 0xFF000000) >> 24) : \
                        \
                        throw("Unsupported sizeof(" #variable ")\n") \
                );
        #define XAL_NORMALIZE_FLOAT_ENDIAN(variable) \
        { \
                uint32_t _var = *(uint32_t*)&variable; \
                XAL_NORMALIZE_ENDIAN(_var); \
                variable = *(float*)&_var; \
        }       
                
#else
        // i386 & friends do a noop
        #define XAL_NORMALIZE_ENDIAN(variable)      
        #define XAL_NORMALIZE_FLOAT_ENDIAN(variable)
#endif

#endif
