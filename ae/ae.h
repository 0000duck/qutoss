/*
* ae.h
* evaluate arithmetic expressions at run time
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 06 May 2010 23:45:53
* This code is hereby placed in the public domain.
*/

#ifndef AE_H
#define AE_H

// "name mangling": .c files use a different compiler than .cpp files,
// so produced symbols will not match (linker assumes everyone is using c++ symbols).
// the ifdef conditions are here to resolve this by informing the linker that we are using C symbols

#ifdef __cplusplus
extern "C" {
#endif

void		ae_open		(void);
void		ae_close	(void);
double		ae_set		(const char* name, double value);
double		ae_eval		(const char* expression);
const char*	ae_error	(void);

#ifdef __cplusplus
}
#endif

/* reference manual

  ae_open()
	Opens ae to be used. Call it once before calling the others.
	Does nothing if ae is already open.

  ae_close()
	Closes ae after use. All variables are deleted.
	Does nothing if ae is already closed.

  ae_set(name,value)
	Sets the value of a variable.
	The value persists until it is set again or ae is closed.

  ae_eval(expression)
	Evaluates the given expression and returns its value.
	Once ae has seen an expression, ae can evaluate it repeatedly quickly.
	Returns 0 if there is an error.  ae_error returns the error message.

  ae_error()
	Returns the last error message or NULL if there is none.
*/

#endif
