/*
 * Copyright 2012 mapx
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Written by Yuchen Xie on 20120421
 */

package mapx.java.soy;

import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.type.TypeReference;

import com.google.common.io.Resources;
import com.google.template.soy.data.SoyMapData;
import com.google.template.soy.SoyFileSet;
import com.google.template.soy.msgs.SoyMsgBundle;
import com.google.template.soy.msgs.SoyMsgBundleHandler;
import com.google.template.soy.tofu.SoyTofu;
import com.google.template.soy.xliffmsgplugin.XliffMsgPlugin;

import java.io.IOException;
import java.util.HashMap;
// jar file iterating
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 * Google closure template handler, the wraper to be called with jni methods.
 */
public final class SoyHandler {
  private final SoyTofu tofu;
  private static final HashMap<String, SoyMsgBundle> msgBundleMap = new HashMap<String, SoyMsgBundle>();
  private static final ObjectMapper mapper = new ObjectMapper();
  private static final TypeReference<HashMap<String, Object>> typeRef = new TypeReference<HashMap<String,Object>>(){};

  /**
   * Render a template with json template name, json data and locale name.
   * @param tplName Template name that starts with dot as written in soy templates.
   * @param jsonMap JSON string that contains data used in the template.
   * @param locale Locale string, "" for default.
   * @return Template String rendered with data in jsonMap.
   * @throws IOException If there is an error reading jsonMap String.
   */
  public String renderToString(String tplName, String jsonMap, String locale) throws IOException {
    HashMap<String,Object> data = mapper.readValue(jsonMap, typeRef);
    return tofu.newRenderer(tplName).setData(data)
      .setMsgBundle(msgBundleMap.get(locale))
      .setIjData(new SoyMapData("locale", locale,
            "tplName", tplName, "data", data))// DEBUG tool
      .render();
  }


  /**
   * Build SoyTofu variable with templates, translations and global_params.txt from this resource jar package.
   * @param resNamespace Namespace these SoyTofu resources in.
   * @throws IOException If there is an error parsing this jar file and open its resources.
   */
  public SoyHandler(String resNamespace) throws IOException {
    String dir = resNamespace + "/templates";
    SoyFileSet.Builder sfsBuilder = new SoyFileSet.Builder();
    String jar = SoyHandler.class.getProtectionDomain().getCodeSource().getLocation().getFile();
    JarFile jf = new JarFile(jar);
    Enumeration<JarEntry> es = jf.entries();
    while (es.hasMoreElements()) {
        String resname = es.nextElement().getName();
        if (resname.startsWith(dir)) {
          if (resname.endsWith(".soy")) { //soy files are the templates
            System.out.println(resname);
            sfsBuilder.add(Resources.getResource(resname));
          } else if (resname.endsWith(".xlf")) { // xlf files are the translations
            String xlfLocale = resname.substring(resname.lastIndexOf("/") + 1, resname.lastIndexOf("."));
            System.out.println(xlfLocale);
            SoyMsgBundleHandler msgBundleHandler = new SoyMsgBundleHandler(new XliffMsgPlugin());
            SoyMsgBundle msgBundle = msgBundleHandler.createFromResource(Resources.getResource(resname));
            msgBundleMap.put(xlfLocale, msgBundle);
          }
        }
    }
    jf.close();
    // and finally a global_params.txt for global params used in all templates.
    tofu = sfsBuilder.setCompileTimeGlobals(Resources.getResource(dir + "/global_params.txt"))
        .build().compileToTofu().forNamespace(resNamespace);
  }

  /**
   * This function stuff an empty string to a jni string outside JVM and simplify ReleaseStringChars calls.
   * @return String("ok")
   */
  public String init() {
    return new String("ok"); 
  }
}
