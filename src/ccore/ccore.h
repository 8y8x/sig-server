#pragma once
#include <node_api.h>

#define NAPI_CALLBACK(name) napi_value (name)(napi_env env, napi_callback_info info)
#define NAPI_OK(op) assert((op) == napi_ok)
