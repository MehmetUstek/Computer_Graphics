#version 410

in vec4 color;
in  vec3 fN;
in  vec3 fL;
in  vec3 fV;
in  vec2 texCoord;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform float Shininess;
uniform int Shading_Mode;
uniform sampler2D tex;
uniform int Drawing_Type;
out vec4 fcolor;

void main()
{

    if (Drawing_Type == 0) {
        fcolor = color;
    }
    else if (Drawing_Type == 1) { // Shading
        if (Shading_Mode == 0) { //Gouraud
            fcolor = color;
        }
        else if (Shading_Mode == 1) {
            //fcolor = color;
            // Normalize the input lighting vectors
            vec3 N = normalize(fN);
            vec3 V = normalize(fV);
            vec3 L = normalize(fL);
            // From class notes: r = 2* (l*n) (n-l)
            vec3 R = normalize(2 * dot(L, N) * (N - L));

            vec4 ambient = AmbientProduct;

            float Kd = max(dot(L, N), 0.0);
            vec4 diffuse = Kd * DiffuseProduct;

            float Ks = pow(max(dot(V, R), 0.0), Shininess);
            vec4 specular = Ks * SpecularProduct;

            // discard the specular highlight if the light's behind the vertex
            if (dot(L, N) < 0.0) {
                specular = vec4(0.0, 0.0, 0.0, 1.0);
            }

            fcolor = ambient + diffuse + specular;
            fcolor.a = 1.0;
        }
        else if (Shading_Mode == 2) {
            //Modified Phong
            //fcolor = color;
           // Normalize the input lighting vectors
            vec3 N = normalize(fN);
            vec3 V = normalize(fV);
            vec3 L = normalize(fL);

            vec3 H = normalize(L + V);

            vec4 ambient = AmbientProduct;

            float Kd = max(dot(L, N), 0.0);
            vec4 diffuse = Kd * DiffuseProduct;

            float Ks = pow(max(dot(N, H), 0.0), Shininess);
            vec4 specular = Ks * SpecularProduct;

            // discard the specular highlight if the light's behind the vertex
            if (dot(L, N) < 0.0) {
                specular = vec4(0.0, 0.0, 0.0, 1.0);
            }

            fcolor = ambient + diffuse + specular;
            fcolor.a = 1.0;
        }
    }
    else if (Drawing_Type == 2) {
        fcolor = texture(tex, texCoord);
        fcolor.a = 1.0;
        //fcolor = vec4(0.5f, 0.5f, 1.0f, 1.0f);

    }
}

