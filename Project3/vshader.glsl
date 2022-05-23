#version 410

in  vec4 vPosition;
in  vec3 vNormal;
in  vec4 vColor;
in  vec2 vTexCoord;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition;
uniform float Shininess;
uniform int Shading_Mode;
uniform int Drawing_Type;
uniform int isLightSourceFixed;


out vec4 color;
out  vec3 fN;
out  vec3 fV;
out  vec3 fL;
out vec2 texCoord;

void main()
{
    
    if (Drawing_Type == 0) {
        color = vColor;
        texCoord = vTexCoord;
        gl_Position = Projection * ModelView * vPosition;
    }
    else if (Drawing_Type == 1) { // Shading
        // ///////////////
        // Gouraud
        // ///////////////
        // Transform vertex position into camera (eye) coordinates
        if (Shading_Mode == 0) {

            vec3 pos = (ModelView * vPosition).xyz;
            vec3 L, N, H, V;

            if (isLightSourceFixed == 1) {
                N = vNormal;
                V = vPosition.xyz;
                L = LightPosition.xyz;
                if (LightPosition.w != 0.0) {
                    L = LightPosition.xyz - vPosition.xyz;
                }
            }
            else {
                L = LightPosition.xyz; // light direction if directional light source
                if (LightPosition.w != 0.0) L = LightPosition.xyz - pos;  // if point light source
                L = normalize(L);
                //vec3 L = normalize( LightPosition.xyz - pos ); //light direction
                V = normalize(-pos); // viewer direction
                H = normalize(L + V); // halfway vector

                // Transform vertex normal into camera coordinates
                N = normalize(ModelView * vec4(vNormal, 0.0)).xyz;

            }

            // Compute terms in the illumination equation
            vec4 ambient = AmbientProduct;

            float Kd = max(dot(L, N), 0.0); //set diffuse to 0 if light is behind the surface point
            vec4  diffuse = Kd * DiffuseProduct;

            float Ks = pow(max(dot(N, H), 0.0), Shininess);
            vec4  specular = Ks * SpecularProduct;

            //ignore also specular component if light is behind the surface point
            if (dot(L, N) < 0.0) {
                specular = vec4(0.0, 0.0, 0.0, 1.0);
            }

            color = vColor;
            texCoord = vTexCoord;
            gl_Position = Projection * ModelView * vPosition;

            color = ambient + diffuse + specular;
            color.a = 1.0;
        }

        // /////////////// 
        // Phong
        // ///////////////
        // Transform vertex position into camera (eye) coordinates
        else if (Shading_Mode == 1) {

            if (isLightSourceFixed == 1) {
                ///// Phong with fixed lighting.
                fN = vNormal;
                fV = vPosition.xyz;
                fL = LightPosition.xyz;
                if (LightPosition.w != 0.0) {
                    fL = LightPosition.xyz - vPosition.xyz;
                }
                gl_Position = Projection * ModelView * vPosition;
            }
            else {
                vec3 pos = (ModelView * vPosition).xyz;

                fN = (ModelView * vec4(vNormal, 0.0)).xyz; // normal direction in camera coordinates

                fV = -pos; //viewer direction in camera coordinates

                fL = LightPosition.xyz; // light direction

                if (LightPosition.w != 0.0) {
                    fL = LightPosition.xyz - pos;  //point light source
                }

                gl_Position = Projection * ModelView * vPosition;
                color = vColor;
            }
        }
    }
    else if (Drawing_Type == 2) {
        color = vColor;
        texCoord = vTexCoord;
        vec3 pos = (ModelView * vPosition).xyz;
        vec3 L;
        L = LightPosition.xyz; // light direction if directional light source
        if (LightPosition.w != 0.0) L = LightPosition.xyz - pos;  // if point light source
  

        L = normalize(L);
        //vec3 L = normalize( LightPosition.xyz - pos ); //light direction
        vec3 V = normalize(-pos); // viewer direction
        vec3 H = normalize(L + V); // halfway vector

        // Transform vertex normal into camera coordinates
        vec3 N = normalize(ModelView * vec4(vNormal, 0.0)).xyz;

        // Compute terms in the illumination equation
        vec4 ambient = AmbientProduct;

        float Kd = max(dot(L, N), 0.0); //set diffuse to 0 if light is behind the surface point
        vec4  diffuse = Kd * DiffuseProduct;

        float Ks = pow(max(dot(N, H), 0.0), Shininess);
        vec4  specular = Ks * SpecularProduct;

        //ignore also specular component if light is behind the surface point
        if (dot(L, N) < 0.0) {
            specular = vec4(0.0, 0.0, 0.0, 1.0);
        }

        gl_Position = Projection * ModelView * vPosition;

        color = ambient + diffuse + specular;
        color.a = 1.0;
    }
}
