#ifndef __JEXCEPTION_H__
#define __JEXCEPTION_H__

#include <stdint.h>
#include <setjmp.h>

#define INIT_EXCEPTION_SYSTEM   jmp_buf __j_exception_buff; volatile uint32_t __last_error_code = 0;

extern jmp_buf __j_exception_buff;
extern volatile uint32_t __last_error_code;

#define GetLastError()  (__last_error_code)

#define try if((__last_error_code = 0), (0 == setjmp(__j_exception_buff)))
#define catch else
#define catchex(ex) else if(__last_error_code == ex)
#define catchgo(lbl) else{goto lbl;}

#define throw(n) {__last_error_code=n; longjmp(__j_exception_buff,1);}

#endif
