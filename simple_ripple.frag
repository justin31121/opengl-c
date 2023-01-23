//http://adrianboeing.blogspot.com/2011/02/ripple-effect-in-webgl.html
#version 330 core

#ifdef GL_ES
precision highp float;
#endif

uniform float time;
uniform sampler2D image;

in vec4 out_color;
in vec2 out_uv;

void main() {    
    vec2 cPos = 2.0*out_uv - 1.0;
    float cLength = length(cPos);

    vec2 img_uv = out_uv + (cPos/cLength)*mix(cos(cLength*12.0-time*4.0)*0.03, 0.0, cLength / 0.25);

    gl_FragColor = texture(image, img_uv);
}
