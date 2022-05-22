#version 410

in vec4 color;
in  vec3 fN;
in  vec3 fL;
in  vec3 fV;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform float Shininess;
uniform int Shading_Mode;

out vec4 fcolor;

void main()
{
     if (Shading_Mode == 0) { //Gouraud
         fcolor = color;
     }
     else if (Shading_Mode == 1) {
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
     else if (Shading_Mode == 2) {
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

