#ifndef __OEM_TOOL_H__
#define __OEM_TOOL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


int oemtool_init(const char *default_xml);



const char *base_model_name();
const char *model_name();
const char *oem_name();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OEM_TOOL_H__ */
