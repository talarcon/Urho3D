#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "Fog.glsl"

varying vec2 vTexCoord;
varying vec4 vWorldPos;
varying vec4 vScreenPos;
#ifdef VERTEXCOLOR
    varying vec4 vColor;
#endif
varying mat4 vModelMatrix;
varying mat4 vModelInv;
varying mat4 vViewInv;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetTexCoord(iTexCoord);
    vScreenPos = gl_Position;
    vWorldPos = vec4(worldPos, 1.0) * cView;
    // vWorldPos = vec4(worldPos, GetDepth(gl_Position));
    vModelMatrix = iModelMatrix;
    // vModelInv = cModelInv;
    vModdlInv = mat4(1.0);
    vViewInv = cViewInv;
    
    #ifdef VERTEXCOLOR
        vColor = iColor;
    #endif
}

void PS()
{
    vec2 resolution = vec2(1.0 / cGBufferInvSize.x, 1.0 / cGBufferInvSize.y);
    vec2 screenPos = vScreenPos.xy / vScreenPos.w;
//     vec2 texCoord = vec2(
//         (1 + screenPos.x) / 2 + 0.5 / resolution.x,
//         (1 - screenPos.y) / 2 + 0.5 / resolution.y
//     );
    vec2 texCoord = vec2(
        screenPos.x / resolution.x,
        screenPos.y / resolution.y
    );
    //Sample a value from the depth buffer
    // vec4 sampledDepth = texture2D(sDepthBuffer, texCoord);
    float depth = ReconstructDepth(texture2D(sDepthBuffer, texCoord).r);
//     float depth = DecodeDepth(sampledDepth.rgb);
    
    vec3 viewRay = vWorldPos.xyz * (cFarClipPS / vWorldPos.z);
    vec3 viewPosition = viewRay * depth;
    
    vec3 worldPosition = (vec4(viewPosition, 1.0) * vViewInv).xyz;
    // vec3 worldPosition = mul(float4(viewPosition, 1), InverseView).xyz;
    vec4 objectPosition = vec4(worldPosition, 1.0) * vModelInv;
    
//     float posAbsX = abs(objectPosition.x);
//     float posAbsY = abs(objectPosition.y);
//     float posAbsZ = abs(objectPosition.z);
//     if (posAbsX < 0.1 || posAbsY < 0.1 || posAbsZ < 0.1)
//         discard;

    gl_FragColor = vec4(viewRay, 1.0);
//     gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    
    // Get material diffuse albedo
//     #ifdef DIFFMAP
//         vec4 diffColor = cMatDiffColor * texture2D(sDiffMap, vTexCoord);
//         #ifdef ALPHAMASK
//             if (diffColor.a < 0.5)
//                 discard;
//         #endif
//     #else
//         vec4 diffColor = cMatDiffColor;
//     #endif
// 
//     #ifdef VERTEXCOLOR
//         diffColor *= vColor;
//     #endif
// 
//     Get fog factor
//     #ifdef HEIGHTFOG
//         float fogFactor = GetHeightFogFactor(vWorldPos.w, vWorldPos.y);
//     #else
//         float fogFactor = GetFogFactor(vWorldPos.w);
//     #endif
// 
//     #if defined(PREPASS)
//         Fill light pre-pass G-Buffer
//         gl_FragData[0] = vec4(0.5, 0.5, 0.5, 1.0);
//         gl_FragData[1] = vec4(EncodeDepth(vWorldPos.w), 0.0);
//     #elif defined(DEFERRED)
//         gl_FragData[0] = vec4(GetFog(diffColor.rgb, fogFactor), diffColor.a);
//         gl_FragData[1] = vec4(0.0, 0.0, 0.0, 0.0);
//         gl_FragData[2] = vec4(0.5, 0.5, 0.5, 1.0);
//         gl_FragData[3] = vec4(EncodeDepth(vWorldPos.w), 0.0);
//     #else
//         gl_FragColor = vec4(GetFog(diffColor.rgb, fogFactor), diffColor.a);
//     #endif
    
//     visualize depthbuffer
//     float x = vScreenPos.x;
//     float y = vScreenPos.y;
//     float depth = DecodeDepth(texture2D(sDepthBuffer, vec2(x, y)).rgb);
//     vec3 color = vec3(depth, depth, depth);
//     gl_FragColor = vec4(color, 1.0);
}
