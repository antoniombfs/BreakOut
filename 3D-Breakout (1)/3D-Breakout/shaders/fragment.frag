#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 VertexColor;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform sampler2D texture1;
uniform int useTexture;

struct MaterialData {
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    float shininess;
};
uniform MaterialData material;

void main() {
    vec3 ambientBase = (material.ambient != vec3(0.0)) ? material.ambient : vec3(0.2, 0.1, 0.4); 
    vec3 ambient = ambientBase * lightColor; 
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor; // A luz agora tem cor!
    
    vec3 specularColor = (material.specular != vec3(0.0)) ? material.specular : vec3(1.0);
    float shininess = (material.shininess > 0.0) ? material.shininess : 32.0;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = 0.8 * spec * specularColor * lightColor; // Brilho também reflete a cor da luz
    
    vec3 baseColor;
    if (useTexture == 1) {
        vec4 texColor = texture(texture1, TexCoords);
        baseColor = texColor.rgb;
    } else {
        if (material.diffuse != vec3(0.0)) baseColor = material.diffuse;
        else if (VertexColor != vec3(0.0)) baseColor = VertexColor;
        else baseColor = objectColor;
    }
    
    vec3 result = (ambient + diffuse + specular) * baseColor;
    FragColor = vec4(result, 1.0);
}