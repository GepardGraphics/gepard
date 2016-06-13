/* Copyright (C) 2015-2016, Gepard Graphics
 * Copyright (C) 2015, Szilard Ledan <szledan@gmail.com>
 * Copyright (C) 2015-2016, Kristof Kosztyo <kkristof@inf.u-szeged.hu>
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

#ifndef GEPARD_H
#define GEPARD_H

namespace gepard {

class GepardEngine;
class Image;
class Surface;

class Gepard {
public:
    explicit Gepard(Surface* surface);
    ~Gepard();

    /**
     * 2. State (W3-2DContext-2015)
     */
    void save();
    void restore();

    /* 5. Building paths (W3-2DContext-2015) */

    /*! \name CanvasPathMethods
     *
     *   <blockquote cite="https://www.w3.org/TR/2dcontext/">
     * A path has a list of zero or more subpaths. Each subpath consists of a
     * list of one or more points, connected by straight or curved lines, and a
     * flag indicating whether the subpath is closed or not. A closed subpath
     * is one where the last point of the subpath is connected to the first
     * point of the subpath by a straight line. Subpaths with fewer than two
     * points are ignored when painting the path.
     *  -- <a href="https://www.w3.org/TR/2dcontext/#building-paths">[W3C-2DContext]</a>
     *   </blockquote>
     *
     * \note The documentation contains quotes from the
     * <a href="https://www.w3.org/TR/2dcontext">W3C-2DContext</a>
     * recommandation.  These are closed in a \c \<blockquote\>.
     *
     */
    /// \{

    /*!
     * \brief Marks the current subpath as closed, and starts a new subpath
     * with a point the same as the start and end of the newly closed subpath.
     * -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-closepath">[W3C-2DContext]</a>
     */
    void closePath();
    /*!
     * \brief Creates a new subpath with the given point.
     *  -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-moveto">[W3C-2DContext]</a>
     * \param x  X-axis value of _end_ point
     * \param y  Y-axis value of _end_ point
     *
     */
    void moveTo(float x, float y);
    /*!
     * \brief Adds the given point to the current subpath, connected to the
     * previous one by a straight line.
     *  -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-lineto">[W3C-2DContext]</a>
     * \param x X-axis value of _end_ point
     * \param y Y-axis value of _end_ point
     */
    void lineTo(float x, float y);
    /*!
     * \brief Adds the given point to the current subpath, connected to the
     * previous one by a quadratic Bézier curve with the given control point.
     *  -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-quadraticcurveto">[W3C-2DContext]</a>
     * \param cpx  X-axis value of _control_ point
     * \param cpy  Y-axis value of _control_ point
     * \param x  X-axis value of _end_ point
     * \param y  Y-axis value of _end_ point
     */
    void quadraticCurveTo(float cpx, float cpy, float x, float y);
    /*!
     * \brief Adds the given point to the current subpath, connected to the
     * previous one by a cubic Bézier curve with the given control points.
     *  -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-beziercurveto">[W3C-2DContext]</a>
     * \param cp1x  X-axis value of _first control_ point
     * \param cp1y  Y-axis value of _first control_ point
     * \param cp2x  X-axis value of _second control_ point
     * \param cp2y  Y-axis value of _second control_ point
     * \param x  X-axis value of _end_ point
     * \param y  Y-axis value of _end_ point
     */
    void bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y);
    /*!
     * \brief Adds an arc with the given control points and radius to the
     * current subpath, connected to the previous point by a straight line.
     *  -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-arcto">[W3C-2DContext]</a>
     * \param x1  X-axis value of _tangent_ point
     * \param y1  Y-axis value of _tangent_ point
     * \param x2  X-axis value of _end_ point
     * \param y2  Y-axis value of _end_ point
     * \param radius  size of arc
     */
    void arcTo(float x1, float y1, float x2, float y2, float radius);
    /*!
     * \brief Adds a new closed subpath to the path, representing the given
     * rectangle.
     *  -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-rect">[W3C-2DContext]</a>
     * \param x  X-axis value of _start_ and _end_ point
     * \param y  Y-axis value of _start_ and _end_ point
     * \param w  size on X-axis
     * \param h  size on Y-axis
     */
    void rect(float x, float y, float w, float h);
    /*!
     * \brief Adds points to the subpath such that the arc described by the
     * circumference of the circle described by the arguments, starting at the
     * given start angle and ending at the given end angle, going in the given
     * direction (defaulting to clockwise), is added to the path, connected to
     * the previous point by a straight line.
     *  -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-arc">[W3C-2DContext]</a>
     * \param x  X-axis value of _center_ point
     * \param y  Y-axis value of _center_ point
     * \param radius  size of arc
     * \param startAngle  specify the _start_ point on arc
     * \param endAngle  specify the _end_ point on arc
     * \param counterclockwise  specify the draw direction on arc
     */
    void arc(float x, float y, float radius, float startAngle, float endAngle, bool counterclockwise = false);
    /// \}

    /**
     * 6. Transformations (W3-2DContext-2015)
     * (default: transform is the identity matrix)
     */
    void scale(float x, float y);
    void rotate(float angle);
    void translate(float x, float y);
    void transform(float a, float b, float c, float d, float e, float f);
    void setTransform(float a, float b, float c, float d, float e, float f);

    /**
     * 9. Rectangles (W3-2DContext-2015)
     */
    void clearRect(float x, float y, float w, float h);
    void fillRect(float x, float y, float w, float h);
    void strokeRect(float x, float y, float w, float h);

    /* 11. Drawing paths to the canvas (W3-2DContext-2015) */

    /*!
     * \brief Resets the current path.
     * -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-beginpath">[W3C-2DContext]</a>
     */
    void beginPath();
    /*!
     * \brief Fills the subpaths of the current path or the given path with the
     * current fill style.
     * -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-fill">[W3C-2DContext]</a>
     */
    void fill();
    /*!
     * \brief Strokes the subpaths of the current path or the given path with
     * the current stroke style.
     * -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-stroke">[W3C-2DContext]</a>
     */
    void stroke();
    /*!
     * \brief Informs the user of the canvas location for the fallback element,
     * based on the current path. If the given element has focus, draws a focus
     * outline around the current path following the platform or user agent
     * conventions for focus outlines as defined by the user agent.
     * -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-drawfocusifneeded">[W3C-2DContext]</a>
     */
    void drawFocusIfNeeded(/*Element element*/);
    /*!
     * \brief Further constrains the clipping region to the current path.
     * -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-clip">[W3C-2DContext]</a>
     */
    void clip();
    /*!
     * \brief Returns true if the given point is in the current path.
     * -- <a href="https://www.w3.org/TR/2dcontext/#dom-context-2d-ispointinpath">[W3C-2DContext]</a>
     * \param x  X-axis value of the given point
     * \param y  Y-axis value of the given point
     * \return  true if the given _point_ is in the current path
     */
    bool isPointInPath(float x, float y);

    /**
     * 12. Drawing images (W3-2DContext-2015)
     */
    void drawImage(Image image, float dx, float dy);
    void drawImage(Image image, float dx, float dy, float dw, float dh);
    void drawImage(Image image, float sx, float sy, float sw, float sh,
        float dx, float dy, float dw, float dh);

    /**
     * 14. Pixel manipulation (W3-2DContext-2015)
     */
    Image createImageData(float sw, float sh);
    Image createImageData(Image imagedata);
    Image getImageData(double sx, double sy, double sw, double sh);
    void putImageData(Image imagedata, double dx, double dy);
    void putImageData(Image imagedata, double dx, double dy, double dirtyX, double dirtyY,
        double dirtyWidth, double dirtyHeight);

private:
    GepardEngine* _engine;
};

} // namespace gepard

#endif // GEPARD_H
