/*! \todo Add license */
# version 450

layout(set=0, binding=0) uniform uArguments
{
    mat4 transformation;
    vec4 fillColor;
} uArgs;

layout(location = 0) out vec4 uFragColor;

void main()
{
    uFragColor = uArgs.fillColor;
}
