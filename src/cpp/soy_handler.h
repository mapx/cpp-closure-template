/**
 * soy_handler.h The adapter between jvm soy handler and native code
 * @ref: http://java.sun.com/docs/books/jni/html/objtypes.html
 * $Date: 2013-06-02 15:54:54 +0800 $
 */

#ifndef MAPX_MODEL_SOY_HANDLER_H
#define MAPX_MODEL_SOY_HANDLER_H

#include <cstring>
#include <cstdlib>

#include <cassert>
#ifndef NDEBUG
#include <cstdio>
#endif

#include <jni.h>

#define MAPX_SOY_JAR_FILE "/path/to/soy_package.jar"
#define MAPX_RESOURCE_NAMESPACE "mapx"
#define MAPX_SOYHANDLER_IN_JAR "mapx/java/soy/SoyHandler"

//#include <ngx_string.h>
typedef struct {
  size_t      len;
  u_char     *data;
} ngx_str_t;


namespace mapx {
  class SoyHandler {
    public:
      SoyHandler() {
        JavaVMOption options[2];
        JavaVMInitArgs vm_args;
        jclass cls;

        options[0].optionString = const_cast<char*>("-Djava.compiler=NONE");
        options[1].optionString = const_cast<char*>(
            "-Djava.class.path=" MAPX_SOY_JAR_FILE);
        //options[2].optionString = "-verbose:jni"; //for debug

        vm_args.version = JNI_VERSION_1_6;
        vm_args.nOptions = 2;
        vm_args.options = options;
        vm_args.ignoreUnrecognized = JNI_TRUE;

        long status = JNI_CreateJavaVM(&m_jvm, (void**)&m_env, &vm_args);
        assert(!status && printf("create java jvm success\n"));
        if (status) {
          exit(-2);
        }
        cls = m_env->FindClass(MAPX_SOYHANDLER_IN_JAR);
        assert(cls && printf("found java class\n"));
        jstring resNamespace = m_env->NewStringUTF(MAPX_RESOURCE_NAMESPACE);
        m_mid = m_env->GetMethodID(cls,"<init>","(Ljava/lang/String;)V");//constructor
        assert(m_mid && printf("Got jvm object constructor\n"));
        m_jobj = m_env->NewObject(cls, m_mid, resNamespace);
#ifndef NDEBUG
        if (m_env->ExceptionCheck()) {
          m_env->ExceptionDescribe();
        }
#endif


        // stuff m_jstrRet and m_ret with useless string for easy call ReleaseStringUTFChars
        m_mid = m_env->GetMethodID(cls, "init", "()Ljava/lang/String;");
        m_jstrRet = static_cast<jstring>(m_env->CallObjectMethod(m_jobj, m_mid));
#ifndef NDEBUG
        if (m_env->ExceptionCheck()) {
          m_env->ExceptionDescribe();
        }
#endif
        assert(m_jstrRet && m_mid && printf("called init method in jvm\n"));
        m_ret.data = (u_char*)(m_env->GetStringUTFChars(m_jstrRet, NULL));
        m_ret.len = m_env->GetStringLength(m_jstrRet);
        assert(printf("-----%s,%zu----\n", m_ret.data, m_ret.len));
        assert(m_ret.len == 2 && m_ret.data && printf("m_ret inited"));

        m_mid = m_env->GetMethodID(cls, "renderToString", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
        assert(m_mid && printf("Got method renderToString\n"));
      }

      void init() {
        // do nothing
      }

      /**
       * Render the template with data in json_map and locale, keep the result inside.
       * @param[in] tpl_name Template name to render.
       * @param[in] json_map data of json map format needed in rendering the template.
       * @param[in] locale string for translation.
       */
      void render(const char* tpl_name, const char* json_map, const char* locale) {
        assert(tpl_name && json_map && locale);
        jstring TplName = m_env->NewStringUTF(tpl_name);
        jstring JsonMap = m_env->NewStringUTF(json_map);
        jstring Locale  = m_env->NewStringUTF(locale);
        // first informs the VM that the native code no longer needs access to m_jstrRet.
        m_env->ReleaseStringUTFChars(m_jstrRet, reinterpret_cast<char*>(m_ret.data));
#ifndef NDEBUG
        // make it unusable
        m_ret.data = NULL;
        m_ret.len = 0;
#endif
        // then render new string to m_jstrRet
        m_jstrRet = static_cast<jstring>(m_env->CallObjectMethod(m_jobj, m_mid, TplName, JsonMap, Locale));
#ifndef NDEBUG
        if (m_env->ExceptionCheck()) {
          m_env->ExceptionDescribe();
        }
#endif
        m_ret.data = (u_char*)(m_env->GetStringUTFChars(m_jstrRet, NULL));
#ifndef NDEBUG
        if (m_env->ExceptionCheck()) {
          m_env->ExceptionDescribe();
        }
#endif
        m_ret.len = m_env->GetStringUTFLength(m_jstrRet);
        assert(m_jstrRet ||
            (printf("DEBUG: response=%s\nDEBUG: len=%zu\n", m_ret.data, m_ret.len)
             && false));
      }

      /**
       * Return the readonly ngx_str_t reference kept inside.
       * This function should be called after render method.
       */
      const ngx_str_t & ngx_str() const {
        return m_ret;
      }

      ~SoyHandler() {
        m_jvm->DestroyJavaVM();
        assert(fprintf(stdout, "Java VM destory.\n"));
      }

    private:
      JNIEnv *m_env;
      JavaVM *m_jvm;
      jobject m_jobj;
      jmethodID m_mid;
      jstring m_jstrRet;
      ngx_str_t m_ret;
  };
}
#undef MAPX_SOY_JAR_FILE
#undef MAPX_RESOURCE_NAMESPACE
#undef MAPX_SOYHANDLER_IN_JAR
#endif

