/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

class ComponentLayout;

//==============================================================================
/**
    A rectangle whose coordinates can be defined in terms of absolute or
    proportional distances.

    Designed mainly for storing component positions, this gives you a lot of
    control over how each coordinate is stored, either as an absolute position,
    or as a proportion of the size of a parent rectangle.

    It also allows you to define the anchor points by which the rectangle is
    positioned, so for example you could specify that the top right of the
    rectangle should be an absolute distance from its parent's bottom-right corner.

    This object can be stored as a string, which takes the form "x y w h", including
    symbols like '%' and letters to indicate the anchor point. See its toString()
    method for more info.

    Example usage:
    @code
    class MyComponent
    {
        void resized()
        {
            // this will set the child component's x to be 20% of our width, its y
            // to be 30, its width to be 150, and its height to be 50% of our
            // height..
            const PositionedRectangle pos1 ("20% 30 150 50%");
            pos1.applyToComponent (*myChildComponent1);

            // this will inset the child component with a gap of 10 pixels
            // around each of its edges..
            const PositionedRectangle pos2 ("10 10 20M 20M");
            pos2.applyToComponent (*myChildComponent2);
        }
    };
    @endcode
*/
class PositionedRectangle
{
public:
    //==============================================================================
    /** Creates an empty rectangle with all coordinates set to zero.

        The default anchor point is top-left; the default
    */
    PositionedRectangle() noexcept
        : x (0.0), y (0.0), w (0.0), h (0.0),
          xMode (anchorAtLeftOrTop | absoluteFromParentTopLeft),
          yMode (anchorAtLeftOrTop | absoluteFromParentTopLeft),
          wMode (absoluteSize), hMode (absoluteSize)
    {
    }

    /** Initialises a PositionedRectangle from a saved string version.

        The string must be in the format generated by toString().
    */
    PositionedRectangle (const String& stringVersion) noexcept
    {
        StringArray tokens;
        tokens.addTokens (stringVersion, false);

        decodePosString (tokens [0], xMode, x);
        decodePosString (tokens [1], yMode, y);
        decodeSizeString (tokens [2], wMode, w);
        decodeSizeString (tokens [3], hMode, h);
    }

    /** Creates a copy of another PositionedRectangle. */
    PositionedRectangle (const PositionedRectangle& other) noexcept
        : x (other.x), y (other.y), w (other.w), h (other.h),
          xMode (other.xMode), yMode (other.yMode),
          wMode (other.wMode), hMode (other.hMode)
    {
    }

    /** Copies another PositionedRectangle. */
    PositionedRectangle& operator= (const PositionedRectangle& other) noexcept
    {
        x = other.x;
        y = other.y;
        w = other.w;
        h = other.h;
        xMode = other.xMode;
        yMode = other.yMode;
        wMode = other.wMode;
        hMode = other.hMode;

        return *this;
    }

    //==============================================================================
    /** Returns a string version of this position, from which it can later be
        re-generated.

        The format is four coordinates, "x y w h".

        - If a coordinate is absolute, it is stored as an integer, e.g. "100".
        - If a coordinate is proportional to its parent's width or height, it is stored
          as a percentage, e.g. "80%".
        - If the X or Y coordinate is relative to the parent's right or bottom edge, the
          number has "R" appended to it, e.g. "100R" means a distance of 100 pixels from
          the parent's right-hand edge.
        - If the X or Y coordinate is relative to the parent's centre, the number has "C"
          appended to it, e.g. "-50C" would be 50 pixels left of the parent's centre.
        - If the X or Y coordinate should be anchored at the component's right or bottom
          edge, then it has "r" appended to it. So "-50Rr" would mean that this component's
          right-hand edge should be 50 pixels left of the parent's right-hand edge.
        - If the X or Y coordinate should be anchored at the component's centre, then it
          has "c" appended to it. So "-50Rc" would mean that this component's
          centre should be 50 pixels left of the parent's right-hand edge. "40%c" means that
          this component's centre should be placed 40% across the parent's width.
        - If it's a width or height that should use the parentSizeMinusAbsolute mode, then
          the number has "M" appended to it.

        To reload a stored string, use the constructor that takes a string parameter.
    */
    String toString() const
    {
        String s;
        s.preallocateBytes (32);
        addPosDescription (s, xMode, x);  s << ' ';
        addPosDescription (s, yMode, y);  s << ' ';
        addSizeDescription (s, wMode, w); s << ' ';
        addSizeDescription (s, hMode, h);
        return s;
    }

    //==============================================================================
    /** Calculates the absolute position, given the size of the space that
        it should go in.

        This will work out any proportional distances and sizes relative to the
        target rectangle, and will return the absolute position.

        @see applyToComponent
    */
    Rectangle<int> getRectangle (const Rectangle<int>& target) const noexcept
    {
        jassert (! target.isEmpty());

        double x_, y_, w_, h_;
        applyPosAndSize (x_, w_, x, w, xMode, wMode, target.getX(), target.getWidth());
        applyPosAndSize (y_, h_, y, h, yMode, hMode, target.getY(), target.getHeight());
        return Rectangle<int> (roundToInt (x_), roundToInt (y_), roundToInt (w_), roundToInt (h_));
    }

    /** Same as getRectangle(), but returning the values as doubles rather than ints. */
    void getRectangleDouble (const Rectangle<int>& target,
                             double& x_, double& y_, double& w_, double& h_) const noexcept
    {
        jassert (! target.isEmpty());
        applyPosAndSize (x_, w_, x, w, xMode, wMode, target.getX(), target.getWidth());
        applyPosAndSize (y_, h_, y, h, yMode, hMode, target.getY(), target.getHeight());
    }

    /** This sets the bounds of the given component to this position.

        This is equivalent to writing:
        @code
        comp.setBounds (getRectangle (Rectangle<int> (0, 0, comp.getParentWidth(), comp.getParentHeight())));
        @endcode

        @see getRectangle, updateFromComponent
    */
    void applyToComponent (Component& comp) const noexcept
    {
        comp.setBounds (getRectangle (Rectangle<int> (comp.getParentWidth(), comp.getParentHeight())));
    }

    //==============================================================================
    /** Updates this object's coordinates to match the given rectangle.

        This will set all coordinates based on the given rectangle, re-calculating
        any proportional distances, and using the current anchor points.

        So for example if the x coordinate mode is currently proportional, this will
        re-calculate x based on the rectangle's relative position within the target
        rectangle's width.

        If the target rectangle's width or height are zero then it may not be possible
        to re-calculate some proportional coordinates. In this case, those coordinates
        will not be changed.
    */
    void updateFrom (const Rectangle<int>& newPosition,
                     const Rectangle<int>& targetSpaceToBeRelativeTo) noexcept
    {
        updatePosAndSize (x, w, newPosition.getX(), newPosition.getWidth(), xMode, wMode, targetSpaceToBeRelativeTo.getX(), targetSpaceToBeRelativeTo.getWidth());
        updatePosAndSize (y, h, newPosition.getY(), newPosition.getHeight(), yMode, hMode, targetSpaceToBeRelativeTo.getY(), targetSpaceToBeRelativeTo.getHeight());
    }

    /** Same functionality as updateFrom(), but taking doubles instead of ints.
    */
    void updateFromDouble (const double newX, const double newY,
                           const double newW, const double newH,
                           const Rectangle<int>& target) noexcept
    {
        updatePosAndSize (x, w, newX, newW, xMode, wMode, target.getX(), target.getWidth());
        updatePosAndSize (y, h, newY, newH, yMode, hMode, target.getY(), target.getHeight());
    }

    /** Updates this object's coordinates to match the bounds of this component.

        This is equivalent to calling updateFrom() with the component's bounds and
        it parent size.

        If the component doesn't currently have a parent, then proportional coordinates
        might not be updated because it would need to know the parent's size to do the
        maths for this.
    */
    void updateFromComponent (const Component& comp) noexcept
    {
        if (comp.getParentComponent() == 0 && ! comp.isOnDesktop())
            updateFrom (comp.getBounds(), Rectangle<int>());
        else
            updateFrom (comp.getBounds(), Rectangle<int> (comp.getParentWidth(), comp.getParentHeight()));
    }

    //==============================================================================
    /** Specifies the point within the rectangle, relative to which it should be positioned. */
    enum AnchorPoint
    {
        anchorAtLeftOrTop              = 1 << 0,    /**< The x or y coordinate specifies where the left or top edge of the rectangle should be. */
        anchorAtRightOrBottom          = 1 << 1,    /**< The x or y coordinate specifies where the right or bottom edge of the rectangle should be. */
        anchorAtCentre                 = 1 << 2     /**< The x or y coordinate specifies where the centre of the rectangle should be. */
    };

    /** Specifies how an x or y coordinate should be interpreted. */
    enum PositionMode
    {
        absoluteFromParentTopLeft       = 1 << 3,   /**< The x or y coordinate specifies an absolute distance from the parent's top or left edge. */
        absoluteFromParentBottomRight   = 1 << 4,   /**< The x or y coordinate specifies an absolute distance from the parent's bottom or right edge. */
        absoluteFromParentCentre        = 1 << 5,   /**< The x or y coordinate specifies an absolute distance from the parent's centre. */
        proportionOfParentSize          = 1 << 6    /**< The x or y coordinate specifies a proportion of the parent's width or height, measured from the parent's top or left. */
    };

    /** Specifies how the width or height should be interpreted. */
    enum SizeMode
    {
        absoluteSize                    = 1 << 0,   /**< The width or height specifies an absolute size. */
        parentSizeMinusAbsolute         = 1 << 1,   /**< The width or height is an amount that should be subtracted from the parent's width or height. */
        proportionalSize                = 1 << 2,   /**< The width or height specifies a proportion of the parent's width or height. */
    };

    //==============================================================================
    /** Sets all options for all coordinates.

        This requires a reference rectangle to be specified, because if you're changing any
        of the modes from proportional to absolute or vice-versa, then it'll need to convert
        the coordinates, and will need to know the parent size so it can calculate this.
    */
    void setModes (const AnchorPoint xAnchor, const PositionMode xMode_,
                   const AnchorPoint yAnchor, const PositionMode yMode_,
                   const SizeMode widthMode, const SizeMode heightMode,
                   const Rectangle<int>& target) noexcept
    {
        if (xMode != (xAnchor | xMode_) || wMode != widthMode)
        {
            double tx, tw;
            applyPosAndSize (tx, tw, x, w, xMode, wMode, target.getX(), target.getWidth());

            xMode = (uint8) (xAnchor | xMode_);
            wMode = (uint8) widthMode;

            updatePosAndSize (x, w, tx, tw, xMode, wMode, target.getX(), target.getWidth());
        }

        if (yMode != (yAnchor | yMode_) || hMode != heightMode)
        {
            double ty, th;
            applyPosAndSize (ty, th, y, h, yMode, hMode, target.getY(), target.getHeight());

            yMode = (uint8) (yAnchor | yMode_);
            hMode = (uint8) heightMode;

            updatePosAndSize (y, h, ty, th, yMode, hMode, target.getY(), target.getHeight());
        }
    }

    /** Returns the anchoring mode for the x coordinate.
        To change any of the modes, use setModes().
    */
    AnchorPoint getAnchorPointX() const noexcept
    {
        return (AnchorPoint) (xMode & (anchorAtLeftOrTop | anchorAtRightOrBottom | anchorAtCentre));
    }

    /** Returns the positioning mode for the x coordinate.
        To change any of the modes, use setModes().
    */
    PositionMode getPositionModeX() const noexcept
    {
        return (PositionMode) (xMode & (absoluteFromParentTopLeft | absoluteFromParentBottomRight
                                         | absoluteFromParentCentre | proportionOfParentSize));
    }

    /** Returns the raw x coordinate.

        If the x position mode is absolute, then this will be the absolute value. If it's
        proportional, then this will be a fractional proportion, where 1.0 means the full
        width of the parent space.
    */
    double getX() const noexcept                    { return x; }

    /** Sets the raw value of the x coordinate.
        See getX() for the meaning of this value.
    */
    void setX (const double newX) noexcept          { x = newX; }

    /** Returns the anchoring mode for the y coordinate.
        To change any of the modes, use setModes().
    */
    AnchorPoint getAnchorPointY() const noexcept
    {
        return (AnchorPoint) (yMode & (anchorAtLeftOrTop | anchorAtRightOrBottom | anchorAtCentre));
    }

    /** Returns the positioning mode for the y coordinate.
        To change any of the modes, use setModes().
    */
    PositionMode getPositionModeY() const noexcept
    {
        return (PositionMode) (yMode & (absoluteFromParentTopLeft | absoluteFromParentBottomRight
                                         | absoluteFromParentCentre | proportionOfParentSize));
    }

    /** Returns the raw y coordinate.

        If the y position mode is absolute, then this will be the absolute value. If it's
        proportional, then this will be a fractional proportion, where 1.0 means the full
        height of the parent space.
    */
    double getY() const noexcept                    { return y; }

    /** Sets the raw value of the y coordinate.
        See getY() for the meaning of this value.
    */
    void setY (const double newY) noexcept          { y = newY; }

    /** Returns the mode used to calculate the width.
        To change any of the modes, use setModes().
    */
    SizeMode getWidthMode() const noexcept          { return (SizeMode) wMode; }

    /** Returns the raw width value.

        If the width mode is absolute, then this will be the absolute value. If the mode is
        proportional, then this will be a fractional proportion, where 1.0 means the full
        width of the parent space.
    */
    double getWidth() const noexcept                { return w; }

    /** Sets the raw width value.

        See getWidth() for the details about what this value means.
    */
    void setWidth (const double newWidth) noexcept  { w = newWidth; }

    /** Returns the mode used to calculate the height.
        To change any of the modes, use setModes().
    */
    SizeMode getHeightMode() const noexcept         { return (SizeMode) hMode; }

    /** Returns the raw height value.

        If the height mode is absolute, then this will be the absolute value. If the mode is
        proportional, then this will be a fractional proportion, where 1.0 means the full
        height of the parent space.
    */
    double getHeight() const noexcept               { return h; }

    /** Sets the raw height value.

        See getHeight() for the details about what this value means.
    */
    void setHeight (const double newHeight) noexcept    { h = newHeight; }

    //==============================================================================
    /** If the size and position are constance, and wouldn't be affected by changes
        in the parent's size, then this will return true.
    */
    bool isPositionAbsolute() const noexcept
    {
        return xMode == absoluteFromParentTopLeft
            && yMode == absoluteFromParentTopLeft
            && wMode == absoluteSize
            && hMode == absoluteSize;
    }

    //==============================================================================
    /** Compares two objects. */
    bool operator== (const PositionedRectangle& other) const noexcept
    {
        return x == other.x && y == other.y
            && w == other.w && h == other.h
            && xMode == other.xMode && yMode == other.yMode
            && wMode == other.wMode && hMode == other.hMode;
    }

    /** Compares two objects. */
    bool operator!= (const PositionedRectangle& other) const noexcept
    {
        return ! operator== (other);
    }

private:
    //==============================================================================
    double x, y, w, h;
    uint8 xMode, yMode, wMode, hMode;

    void addPosDescription (String& s, const uint8 mode, const double value) const noexcept
    {
        if ((mode & proportionOfParentSize) != 0)
        {
            s << (roundToInt (value * 100000.0) / 1000.0) << '%';
        }
        else
        {
            s << (roundToInt (value * 100.0) / 100.0);

            if ((mode & absoluteFromParentBottomRight) != 0)
                s << 'R';
            else if ((mode & absoluteFromParentCentre) != 0)
                s << 'C';
        }

        if ((mode & anchorAtRightOrBottom) != 0)
            s << 'r';
        else if ((mode & anchorAtCentre) != 0)
            s << 'c';
    }

    void addSizeDescription (String& s, const uint8 mode, const double value) const noexcept
    {
        if (mode == proportionalSize)
            s << (roundToInt (value * 100000.0) / 1000.0) << '%';
        else if (mode == parentSizeMinusAbsolute)
            s << (roundToInt (value * 100.0) / 100.0) << 'M';
        else
            s << (roundToInt (value * 100.0) / 100.0);
    }

    void decodePosString (const String& s, uint8& mode, double& value) noexcept
    {
        if (s.containsChar ('r'))
            mode = anchorAtRightOrBottom;
        else if (s.containsChar ('c'))
            mode = anchorAtCentre;
        else
            mode = anchorAtLeftOrTop;

        if (s.containsChar ('%'))
        {
            mode |= proportionOfParentSize;
            value = s.removeCharacters ("%rcRC").getDoubleValue() / 100.0;
        }
        else
        {
            if (s.containsChar ('R'))
                mode |= absoluteFromParentBottomRight;
            else if (s.containsChar ('C'))
                mode |= absoluteFromParentCentre;
            else
                mode |= absoluteFromParentTopLeft;

            value = s.removeCharacters ("rcRC").getDoubleValue();
        }
    }

    void decodeSizeString (const String& s, uint8& mode, double& value) noexcept
    {
        if (s.containsChar ('%'))
        {
            mode = proportionalSize;
            value = s.upToFirstOccurrenceOf ("%", false, false).getDoubleValue() / 100.0;
        }
        else if (s.containsChar ('M'))
        {
            mode = parentSizeMinusAbsolute;
            value = s.getDoubleValue();
        }
        else
        {
            mode = absoluteSize;
            value = s.getDoubleValue();
        }
    }

    void applyPosAndSize (double& xOut, double& wOut, const double x_, const double w_,
                          const uint8 xMode_, const uint8 wMode_,
                          const int parentPos, const int parentSize) const noexcept
    {
        if (wMode_ == proportionalSize)
            wOut = roundToInt (w_ * parentSize);
        else if (wMode_ == parentSizeMinusAbsolute)
            wOut = jmax (0, parentSize - roundToInt (w_));
        else
            wOut = roundToInt (w_);

        if ((xMode_ & proportionOfParentSize) != 0)
            xOut = parentPos + x_ * parentSize;
        else if ((xMode_ & absoluteFromParentBottomRight) != 0)
            xOut = (parentPos + parentSize) - x_;
        else if ((xMode_ & absoluteFromParentCentre) != 0)
            xOut = x_ + (parentPos + parentSize / 2);
        else
            xOut = x_ + parentPos;

        if ((xMode_ & anchorAtRightOrBottom) != 0)
            xOut -= wOut;
        else if ((xMode_ & anchorAtCentre) != 0)
            xOut -= wOut / 2;
    }

    void updatePosAndSize (double& xOut, double& wOut, double x_, const double w_,
                           const uint8 xMode_, const uint8 wMode_,
                           const int parentPos, const int parentSize) const noexcept
    {
        if (wMode_ == proportionalSize)
        {
            if (parentSize > 0)
                wOut = w_ / parentSize;
        }
        else if (wMode_ == parentSizeMinusAbsolute)
            wOut = parentSize - w_;
        else
            wOut = w_;

        if ((xMode_ & anchorAtRightOrBottom) != 0)
            x_ += w_;
        else if ((xMode_ & anchorAtCentre) != 0)
            x_ += w_ / 2;

        if ((xMode_ & proportionOfParentSize) != 0)
        {
            if (parentSize > 0)
                xOut = (x_ - parentPos) / parentSize;
        }
        else if ((xMode_ & absoluteFromParentBottomRight) != 0)
            xOut = (parentPos + parentSize) - x_;
        else if ((xMode_ & absoluteFromParentCentre) != 0)
            xOut = x_ - (parentPos + parentSize / 2);
        else
            xOut = x_ - parentPos;
    }
};

//==============================================================================
struct RelativePositionedRectangle
{
    //==============================================================================
    RelativePositionedRectangle()
        : relativeToX (0),
          relativeToY (0),
          relativeToW (0),
          relativeToH (0)
    {
    }

    RelativePositionedRectangle (const RelativePositionedRectangle& other)
        : rect (other.rect),
          relativeToX (other.relativeToX),
          relativeToY (other.relativeToY),
          relativeToW (other.relativeToW),
          relativeToH (other.relativeToH)
    {
    }

    RelativePositionedRectangle& operator= (const RelativePositionedRectangle& other)
    {
        rect = other.rect;
        relativeToX = other.relativeToX;
        relativeToY = other.relativeToY;
        relativeToW = other.relativeToW;
        relativeToH = other.relativeToH;
        return *this;
    }

    //==============================================================================
    bool operator== (const RelativePositionedRectangle& other) const noexcept
    {
        return rect == other.rect
            && relativeToX == other.relativeToX
            && relativeToY == other.relativeToY
            && relativeToW == other.relativeToW
            && relativeToH == other.relativeToH;
    }

    bool operator!= (const RelativePositionedRectangle& other) const noexcept
    {
        return ! operator== (other);
    }

    template <typename LayoutType>
    void getRelativeTargetBounds (const Rectangle<int>& parentArea,
                                  const LayoutType* layout,
                                  int& x, int& xw, int& y, int& yh,
                                  int& w, int& h) const
    {
        Component* rx = 0;
        Component* ry = 0;
        Component* rw = 0;
        Component* rh = 0;

        if (layout != 0)
        {
            rx = layout->findComponentWithId (relativeToX);
            ry = layout->findComponentWithId (relativeToY);
            rw = layout->findComponentWithId (relativeToW);
            rh = layout->findComponentWithId (relativeToH);
        }

        x = parentArea.getX() + (rx != 0 ? rx->getX() : 0);
        y = parentArea.getY() + (ry != 0 ? ry->getY() : 0);
        w = rw != 0 ? rw->getWidth() : parentArea.getWidth();
        h = rh != 0 ? rh->getHeight() : parentArea.getHeight();
        xw = rx != 0 ? rx->getWidth() : parentArea.getWidth();
        yh = ry != 0 ? ry->getHeight() : parentArea.getHeight();
    }

    Rectangle<int> getRectangle (const Rectangle<int>& parentArea,
                                 const ComponentLayout* layout) const
    {
        int x, xw, y, yh, w, h;
        getRelativeTargetBounds (parentArea, layout, x, xw, y, yh, w, h);

        const Rectangle<int> xyRect ((xw <= 0 || yh <= 0) ? Rectangle<int>()
                                                          : rect.getRectangle (Rectangle<int> (x, y, xw, yh)));

        const Rectangle<int> whRect ((w <= 0 || h <= 0) ? Rectangle<int>()
                                                        : rect.getRectangle (Rectangle<int> (x, y, w, h)));

        return Rectangle<int> (xyRect.getX(), xyRect.getY(),
                               whRect.getWidth(), whRect.getHeight());
    }

    void getRectangleDouble (double& x, double& y, double& w, double& h,
                             const Rectangle<int>& parentArea,
                             const ComponentLayout* layout) const
    {
        int rx, rxw, ry, ryh, rw, rh;
        getRelativeTargetBounds (parentArea, layout, rx, rxw, ry, ryh, rw, rh);

        double dummy1, dummy2;
        rect.getRectangleDouble (Rectangle<int> (rx, ry, rxw, ryh), x, y, dummy1, dummy2);
        rect.getRectangleDouble (Rectangle<int> (rx, ry, rw, rh), dummy1, dummy2, w, h);
    }

    void updateFromComponent (const Component& comp, const ComponentLayout* layout)
    {
        int x, xw, y, yh, w, h;
        getRelativeTargetBounds (Rectangle<int> (0, 0, comp.getParentWidth(), comp.getParentHeight()),
                                 layout, x, xw, y, yh, w, h);

        PositionedRectangle xyRect (rect), whRect (rect);
        xyRect.updateFrom (comp.getBounds(), Rectangle<int> (x, y, xw, yh));
        whRect.updateFrom (comp.getBounds(), Rectangle<int> (x, y, w, h));

        rect.setX (xyRect.getX());
        rect.setY (xyRect.getY());
        rect.setWidth (whRect.getWidth());
        rect.setHeight (whRect.getHeight());
    }

    void updateFrom (double newX, double newY, double newW, double newH,
                     const Rectangle<int>& parentArea, const ComponentLayout* layout)
    {
        int x, xw, y, yh, w, h;
        getRelativeTargetBounds (parentArea, layout, x, xw, y, yh, w, h);

        PositionedRectangle xyRect (rect), whRect (rect);
        xyRect.updateFromDouble (newX, newY, newW, newH, Rectangle<int> (x, y, xw, yh));
        whRect.updateFromDouble (newX, newY, newW, newH, Rectangle<int> (x, y, w, h));

        rect.setX (xyRect.getX());
        rect.setY (xyRect.getY());
        rect.setWidth (whRect.getWidth());
        rect.setHeight (whRect.getHeight());
    }

    void applyToXml (XmlElement& e) const
    {
        e.setAttribute ("pos", rect.toString());

        if (relativeToX != 0)   e.setAttribute ("posRelativeX", String::toHexString (relativeToX));
        if (relativeToY != 0)   e.setAttribute ("posRelativeY", String::toHexString (relativeToY));
        if (relativeToW != 0)   e.setAttribute ("posRelativeW", String::toHexString (relativeToW));
        if (relativeToH != 0)   e.setAttribute ("posRelativeH", String::toHexString (relativeToH));
    }

    void restoreFromXml (const XmlElement& e, const RelativePositionedRectangle& defaultPos)
    {
        rect = PositionedRectangle (e.getStringAttribute ("pos", defaultPos.rect.toString()));
        relativeToX = e.getStringAttribute ("posRelativeX", String::toHexString (defaultPos.relativeToX)).getHexValue64();
        relativeToY = e.getStringAttribute ("posRelativeY", String::toHexString (defaultPos.relativeToY)).getHexValue64();
        relativeToW = e.getStringAttribute ("posRelativeW", String::toHexString (defaultPos.relativeToW)).getHexValue64();
        relativeToH = e.getStringAttribute ("posRelativeH", String::toHexString (defaultPos.relativeToH)).getHexValue64();
    }

    String toString() const
    {
        StringArray toks;
        toks.addTokens (rect.toString(), false);

        return toks[0] + " " + toks[1];
    }

    Point<float> toXY (const Rectangle<int>& parentArea,
                       const ComponentLayout* layout) const
    {
        double x, y, w, h;
        getRectangleDouble (x, y, w, h, parentArea, layout);
        return Point<float> ((float) x, (float) y);
    }

    void getXY (double& x, double& y,
                const Rectangle<int>& parentArea,
                const ComponentLayout* layout) const
    {
        double w, h;
        getRectangleDouble (x, y, w, h, parentArea, layout);
    }

    //==============================================================================
    PositionedRectangle rect;
    int64 relativeToX;
    int64 relativeToY;
    int64 relativeToW;
    int64 relativeToH;
};
