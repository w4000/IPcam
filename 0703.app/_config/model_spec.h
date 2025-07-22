#ifndef __MODEL_SPEC_H__
#define __MODEL_SPEC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

int model_spec_reload();
int model_spec_all(char *str);
const char *model_spec_string(const char *key);
const int model_spec_int(const char *key);

const char *base_model_name();
const char *model_name();
const char *oem_name();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __MODEL_SPEC_H__ */
