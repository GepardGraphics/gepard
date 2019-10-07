/* Copyright (C) 2018, Gepard Graphics
 * Copyright (C) 2018, Szilard Ledan <szledan@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gepard-gles2.h"

#include "gepard-float.h"
#include "gepard-logging.h"
#include <cmath>

namespace gepard {
namespace gles2 {

static const std::string s_textureVertexShader = GD_GLES2_SHADER_PROGRAM(
    precision highp float;

    uniform vec2 u_viewportSize;

    attribute vec4 a_position;

    varying vec2 v_texturePosition;

    void main()
    {
        v_texturePosition = a_position.zw;
        gl_Position = vec4((2.0 * a_position.xy / u_viewportSize) - 1.0, 0.0, 1.0);
    }
);

static const std::string s_textureFragmentShader = GD_GLES2_SHADER_PROGRAM(
    precision highp float;

    uniform sampler2D u_texture;

    varying vec2 v_texturePosition;

    void main()
    {
        gl_FragColor = texture2D(u_texture, v_texturePosition);
    }
);

/*!
 * \brief GepardGLES2::drawImage
 * \param imagedata
 * \param sx
 * \param sy
 * \param sw
 * \param sh
 * \param dx
 * \param dy
 * \param dw
 * \param dh
 *
 * \internal
 * \todo documentation is missing
 */
void GepardGLES2::drawImage(const Image& imageData, const Float sx, const Float sy, const Float sw, const Float sh, const Float dx, const Float dy, const Float dw, const Float dh)
{
    if (!imageData.width() || !imageData.height())
        return;

    const FloatPoint dtl = _context.currentState().transform.apply(FloatPoint(dx, dy));
    const FloatPoint dtr = _context.currentState().transform.apply(FloatPoint(dx + dw, dy));
    const FloatPoint dbl = _context.currentState().transform.apply(FloatPoint(dx, dy + dh));
    const FloatPoint dbr = _context.currentState().transform.apply(FloatPoint(dx + dw, dy + dh));

    makeCurrent();
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    drawImage(imageData, FloatPoint(sx, sy), FloatPoint(sw, sh), dtl, dtr, dbl, dbr);

    render();
}

/*!
 * \brief GepardGLES2::putImage
 * \param imagedata
 * \param dx
 * \param dy
 * \param dirtyX
 * \param dirtyY
 * \param dirtyWidth
 * \param dirtyHeight
 *
 * \internal
 * \todo documentation is missing
 */
void GepardGLES2::putImage(const Image& imageData, const Float dx, const Float dy, const Float dirtyX, const Float dirtyY, const Float dirtyWidth, const Float dirtyHeight)
{
    if (!imageData.width() || !imageData.height())
        return;

    const Float width(std::min((Float)imageData.width(), dirtyWidth));
    const Float height(std::min((Float)imageData.height(), dirtyHeight));
    const FloatPoint dtl(dx, dy);
    const FloatPoint dtr(dx + width, dy);
    const FloatPoint dbl(dx, dy + height);
    const FloatPoint dbr(dx + width, dy + height);

    makeCurrent();
    glBlendFunc(GL_ONE, GL_ZERO);

    drawImage(imageData, FloatPoint(dirtyX, dirtyY), FloatPoint(width, height), dtl, dtr, dbl, dbr);

    render();
}

/*!
 * \brief GepardGLES2::getImage
 * \param sx
 * \param sy
 * \param sw
 * \param sh
 * \return
 *
 * \internal
 * \todo documentation is missing
 */
const Image GepardGLES2::getImage(const Float sx, const Float sy, const Float sw, const Float sh)
{
    std::vector<uint32_t> data(sw * sh);

    makeCurrent();
    glBindFramebuffer(GL_FRAMEBUFFER, _fboId);
    glReadPixels(sx, sy, sw, sh, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) data.data());

    return Image(sw, sh, data);
}

void GepardGLES2::drawImage(const Image& imageData, const FloatPoint srcXY, const FloatPoint srcWH, const FloatPoint destTL, const FloatPoint destTR, const FloatPoint destBL, const FloatPoint destBR)
{
    const uint32_t imgWidth = imageData.width();
    const uint32_t imgHeight = imageData.height();
    GD_ASSERT(imgWidth && imgHeight);

    GLuint imgTextureId;

    makeCurrent();

    glGenTextures(1, &imgTextureId);
    glBindTexture(GL_TEXTURE_2D, imgTextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data().data());

    const uint32_t srfWidth = _context.surface->width();
    const uint32_t srfHeight = _context.surface->height();

    ShaderProgram& textureProgram = _shaderProgramManager.getProgram("s_imgTextureProgram", s_textureVertexShader, s_textureFragmentShader);
    glUseProgram(textureProgram.id);

    {
        const GLfloat textureCoords[] = {
            (GLfloat)destTL.x, (GLfloat)destTL.y, (GLfloat)(srcXY.x / Float(imgWidth)), (GLfloat)(srcXY.y / Float(imgHeight)),
            (GLfloat)destTR.x, (GLfloat)destTR.y, (GLfloat)((srcXY.x + srcWH.x) / Float(imgWidth)), (GLfloat)(srcXY.y / Float(imgHeight)),
            (GLfloat)destBL.x, (GLfloat)destBL.y, (GLfloat)(srcXY.x / Float(imgWidth)), (GLfloat)((srcXY.y + srcWH.y) / Float(imgHeight)),
            (GLfloat)destBR.x, (GLfloat)destBR.y, (GLfloat)((srcXY.x + srcWH.x) / Float(imgWidth)), (GLfloat)((srcXY.y + srcWH.y) / Float(imgHeight)),
        };

        const GLint index = glGetAttribLocation(textureProgram.id, "a_position");
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, 4, GL_FLOAT, GL_FALSE, 0, textureCoords);
    }

    {
        const GLint index = glGetUniformLocation(textureProgram.id, "u_viewportSize");
        glUniform2f(index, srfWidth, srfHeight);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _fboId);
    {
        glActiveTexture(GL_TEXTURE0);
        const GLint index = glGetUniformLocation(textureProgram.id, "u_texture");
        glUniform1i(index, GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, imgTextureId);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDeleteTextures(1, &imgTextureId);
}

} // namespace gles2
} // namespace gepard
