// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// interpolated colour received from vertex stage


in vec3 vecPos;
in vec3 viewPos;
// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

in vec3 FragNormal;

void main(void)
{

//shading variables (coefficients and phong exponent(p))
	float ka = 0.8f;
	float kd = 0.5f;
	float ks = 0.2f;
	float p = 64.0;

	float intensity = 10.f;

   //vec4 color = vec4(FragNormal,1);

   vec4 color = vec4(0.6,0.6,0.6, 1);

    vec3 normal = normalize(FragNormal);
    vec3 lightColor = vec3(1.0);

    // Ambient
    vec3 ambient = ka * color.rgb;
	
	// Diffuse
    vec3 lightDir = normalize(vec3(-50,100,0) - vecPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    
	// Specular
    vec3 viewDir = normalize(viewPos - vecPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    //spec = pow(max(dot(normal, halfwayDir), 0.0), p);
    vec3 specular = spec * lightColor;    


    vec3 lighting = (ambient + (diffuse + specular)) * color.rgb;    
  
	FragmentColour = vec4(lighting, color.a);




	//FragmentColour = vec4(FragNormal, 1.0);
	//FragmentColour = vec4(0,0,1,1);
}
