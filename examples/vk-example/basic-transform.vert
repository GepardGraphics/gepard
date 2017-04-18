/*! \todo Add license */
# version 450

layout(set=0, binding=0) uniform uArguments
{
    mat4 transformation;
    vec4 fillColor;
} uArgs;

layout(location = 0) in vec4 position;

void main()
{
    gl_Position = uArgs.transformation * position;
}
