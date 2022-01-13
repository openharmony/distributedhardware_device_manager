#ifndef PARAMETER__H
#define PARAMETER__H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


int GetParameter(const char *key, const char *def, char *value, unsigned int len);

int SetParameter(const char *key, const char *value);

typedef void (*ParameterChgPtr)(const char *key, const char *value, void *context);
int WatchParameter(const char *keyprefix, ParameterChgPtr callback, void *context);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif