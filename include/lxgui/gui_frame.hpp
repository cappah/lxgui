#ifndef GUI_FRAME_HPP
#define GUI_FRAME_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_region.hpp"

#include <set>
#include <functional>

namespace gui
{
    class backdrop;

    /// Contains layered_region
    struct layer
    {
        layer();

        bool                         bDisabled;
        std::vector<layered_region*> lRegionList;

        static layer_type get_layer_type(const std::string& sLayer);
    };

    /// GUI container
    /** Can contain other frames, or layered_regions
    *   (text, images, ...).
    */
    class frame : public event_receiver, public region
    {
    public :

        /// Constructor.
        explicit frame(manager* pManager);

        /// Destructor.
        virtual ~frame();

        /// Renders this widget on the current render target.
        virtual void render();

        /// updates this widget's logic.
        virtual void update(float fDelta);

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        virtual std::string serialize(const std::string& sTab) const;

        /// Returns 'true' if this frame can use a script.
        /** \param sScriptName The name of the script
        *   \note This method can be overriden if needed.
        */
        virtual bool can_use_script(const std::string& sScriptName) const;

        /// Checks if this frame's position is valid.
        void check_position() const;

        /// Copies an uiobject's parameters into this frame (inheritance).
        /** \param pObj The uiobject to copy
        */
        virtual void copy_from(uiobject* pObj);

        /// Creates a new title region for this frame.
        /** \note You can get it by calling get_title_region().
        */
        void create_title_region();

        /// Disables a layer.
        /** \param mLayerID The id of the layer to disable
        */
        void disable_draw_layer(layer_type mLayerID);

        /// Enables a layer.
        /** \param mLayerID The id of the layer to enable
        */
        void enable_draw_layer(layer_type mLayerID);

        /// Sets if this frame can receive keyboard input.
        /** \param bIsKeyboardEnabled 'true' to enable
        */
        virtual void enable_keyboard(bool bIsKeyboardEnabled);

        /// Sets if this frame can receive mouse input.
        /** \param bIsMouseEnabled 'true' to enable
        *   \param bAllowWorldInput 'true' to allow world input
        */
        virtual void enable_mouse(bool bIsMouseEnabled, bool bAllowWorldInput = false);

        /// Sets if this frame can receive mouse wheel input.
        /** \param bIsMouseWheelEnabled 'true' to enable
        */
        virtual void enable_mouse_wheel(bool bIsMouseWheelEnabled);

        /// Checks if this frame has a script defined.
        /** \param sScriptName The name of the script to check
        *   \return 'true' if this script is defined
        */
        bool has_script(const std::string& sScriptName) const;

        /// Adds a layered_region to this frame's children.
        /** \param pRegion The layered_region to add
        */
        void add_region(layered_region* pRegion);

        /// Removes a layered_region from this frame's children.
        /** \param pRegion The layered_region to remove
        */
        void remove_region(layered_region* pRegion);

        /// Creates a new region as child of this frame.
        /** \param mLayer       The layer on which to create the region
        *   \param sClassName   The name of the region class ("FontString", ...)
        *   \param sName        The name of the region
        *   \param sInheritance The name of the region to inherit from
        *                       (empty if none)
        *   \return The created region.
        *   \note You don't have the reponsability to delete this region.
        *         It will be done automatically when its parent is deleted.
        *   \note This function takes care of the basic initializing :
        *         you can directly use the created region.
        */
        layered_region* create_region(
            layer_type mLayer, const std::string& sClassName,
            const std::string& sName, const std::string& sInheritance = ""
        );

        /// Creates a new region as child of this frame.
        /** \param mLayer       The layer on which to create the region
        *   \param sName        The name of the region
        *   \param sInheritance The name of the region to inherit from
        *                       (empty if none)
        *   \return The created region.
        *   \note You don't have the reponsability to delete this region.
        *         It will be done automatically when its parent is deleted.
        *   \note This function takes care of the basic initializing :
        *         you can directly use the created region.
        */
        #ifndef NO_CPP11_FUNCTION_TEMPLATE_DEFAULT
        template<typename region_type, typename enable = typename std::enable_if<std::is_base_of<gui::layered_region, region_type>::value>::type>
        #else
        template<typename region_type>
        #endif
        region_type* create_region(layer_type mLayer, const std::string& sName, const std::string& sInheritance = "")
        {
            return dynamic_cast<region_type*>(create_region(mLayer, region_type::CLASS_NAME, sName, sInheritance));
        }

        /// Creates a new frame as child of this frame.
        /** \param sClassName   The name of the frame class ("Button", ...)
        *   \param sName        The name of the frame
        *   \param sInheritance The name of the frame to inherit from
        *                       (empty if none)
        *   \return The created frame.
        *   \note You don't have the reponsability to delete this frame.
        *         It will be done automatically when its parent is deleted.
        *   \note This function takes care of the basic initializing :
        *         you can directly use the created frame.
        */
        frame* create_child(const std::string& sClassName, const std::string& sName, const std::string& sInheritance = "");

        /// Creates a new frame as child of this frame.
        /** \param sName        The name of the frame
        *   \param sInheritance The name of the frame to inherit from
        *                       (empty if none)
        *   \return The created frame.
        *   \note You don't have the reponsability to delete this frame.
        *         It will be done automatically when its parent is deleted.
        *   \note This function takes care of the basic initializing :
        *         you can directly use the created frame.
        */
        #ifndef NO_CPP11_FUNCTION_TEMPLATE_DEFAULT
        template<typename frame_type, typename enable = typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        #else
        template<typename frame_type>
        #endif
        frame_type* create_child(const std::string& sName, const std::string& sInheritance = "")
        {
            return dynamic_cast<frame_type*>(create_child(frame_type::CLASS_NAME, sName, sInheritance));
        }

        /// Adds a frame to this frame's children.
        /** \param pChild The frame to add
        */
        void add_child(frame* pChild);

        /// Removes a frame from this frame's children.
        /** \param pChild The frame to remove
        */
        void remove_child(frame* pChild);

        /// Returns the child list.
        /** \return The child list
        */
        const std::map<uint, frame*>& get_children() const;

        /// Returns one of this frame's children.
        /** \param sName The name of the child
        *   \return One of this frame's children
        *   \note The provided name can either be the full name or the relative name
        *         (i.e. without the "$parent" in front). This function first looks
        *         for matches on the full name, then if no child is found, on the
        *         relative name.
        */
        frame* get_child(const std::string& sName);

        /// Returns one of this frame's children.
        /** \param sName The name of the child
        *   \return One of this frame's children
        *   \note The provided name can either be the full name or the relative name
        *         (i.e. without the "$parent" in front). This function first looks
        *         for matches on the full name, then if no child is found, on the
        *         relative name.
        */
        #ifndef NO_CPP11_FUNCTION_TEMPLATE_DEFAULT
        template<typename frame_type, typename enable = typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        #else
        template<typename frame_type>
        #endif
        frame_type* get_child(const std::string& sName)
        {
            return dynamic_cast<frame_type*>(get_child(sName));
        }

        /// Returns one of this frame's region.
        /** \param sName The name of the region
        *   \return One of this frame's region
        *   \note The provided name can either be the full name or the relative name
        *         (i.e. without the "$parent" in front). This function first looks
        *         for matches on the full name, then if no region is found, on the
        *         relative name.
        */
        layered_region* get_region(const std::string& sName);

        /// Returns one of this frame's region.
        /** \param sName The name of the region
        *   \return One of this frame's region
        *   \note The provided name can either be the full name or the relative name
        *         (i.e. without the "$parent" in front). This function first looks
        *         for matches on the full name, then if no region is found, on the
        *         relative name.
        */
        #ifndef NO_CPP11_FUNCTION_TEMPLATE_DEFAULT
        template<typename region_type, typename enable = typename std::enable_if<std::is_base_of<gui::layered_region, region_type>::value>::type>
        #else
        template<typename region_type>
        #endif
        region_type* get_region(const std::string& sName)
        {
            return dynamic_cast<region_type*>(get_region(sName));
        }

        /// Calculates effective alpha.
        /** \return Effective alpha (alpha*parent->alpha)
        */
        float get_effective_alpha() const;

        /// Calculates effective scale.
        /** \return Effective scale (scale*parent->scale)
        */
        float get_effective_scale() const;

        /// Returns this frame's level.
        /** \return This frame's level
        */
        int get_frame_level() const;

        /// Returns this frame's strata.
        /** \return This frame's strata
        */
        frame_strata get_frame_strata() const;

        /// Returns this frame's backdrop.
        /** \return This frame's backdrop
        */
        utils::wptr<backdrop> get_backdrop() const;

        /// Returns this frame's type.
        /** \return This frame's type (Frame, Slider, ...)
        */
        const std::string& get_frame_type() const;

        /// Returns this frame's absolute hit rect insets.
        /** \return This frame's absolute hit rect insets
        */
        const quad2i& get_abs_hit_rect_insets() const;

        /// Returns this frame's relative hit rect insets.
        /** \return This frame's relative hit rect insets
        */
        const quad2f& get_rel_hit_rect_insets() const;

        /// Returns this frame's max dimensions.
        /** \return This frame's max dimensions
        */
        vector2ui get_max_resize() const;

        /// Returns this frame's min dimensions.
        /** \return This frame's min dimensions
        */
        vector2ui get_min_resize() const;

        /// Returns the number of children of this frame.
        /** \return The number of children of this frame
        */
        uint get_num_children() const;

        /// Returns the number of region of this frame.
        /** \return The number of region of this frame
        */
        uint get_num_regions() const;

        /// Returns this frame's scale.
        /** \return This frame's scale
        *   \note If you want it's true scale on the screen,
        *         use get_effective_scale().
        */
        float get_scale() const;

        /// Returns this frame's title region.
        region* get_title_region() const;

        /// Checks if this frame is clamped to screen.
        /** \return 'true' if this frame is clamed to screen
        */
        bool is_clamped_to_screen() const;

        /// Checks if the provided coordinates are in the frame.
        /** \param iX The horizontal coordinate
        *   \param iY The vertical coordinate
        *   \return 'true' if the provided coordinates are in the frame
        */
        virtual bool is_in_frame(int iX, int iY) const;

        /// Checks if this frame can receive keyboard input.
        /** \return 'true' if this frame can receive keyboard input
        */
        bool is_keyboard_enabled() const;

        /// Checks if this frame can receive mouse input.
        /** \return 'true' if this frame can receive mouse input
        */
        bool is_mouse_enabled() const;

        /// Checks if this frame allows world input.
        /** \return 'true' if this frame allows world input
        */
        bool is_world_input_allowed() const;

        /// Checks if this frame can receive mouse wheel input.
        /** \return 'true' if this frame can receive mouse wheel input
        */
        bool is_mouse_wheel_enabled() const;

        /// Checks if this frame can be moved.
        /** \return 'true' if this frame can be moved
        */
        bool is_movable() const;

        /// Checks if this frame can be resized.
        /** \return 'true' if this frame can be resized
        */
        bool is_resizable() const;

        /// Checks if this frame is at top level.
        /** \return 'true' if this frame is at top level
        */
        bool is_top_level() const;

        /// Checks if this frame has been moved by the user.
        /** \return 'true' if this frame has been moved by the user
        */
        bool is_user_placed() const;

        /// Registers a handler script to this frame.
        /** \param sScriptName The name of the script
        *   \param sContent    The content ot the script
        *   \param sFile       The file in which this script has been found
        *   \param uiLineNbr   The line number at which this script is located in the file
        *   \note The last two informations are required for error messages.
        */
        void define_script(const std::string& sScriptName, const std::string& sContent,
            const std::string& sFile, uint uiLineNbr);

        typedef std::function<void(frame*, event*)> handler;

        /// Registers a handler script to this frame.
        /** \param sScriptName The name of the script
        *   \param mHandler    The handler ot the script
        */
        void define_script(const std::string& sScriptName, handler mHandler);

        /// Tells this frame that a script has been defined.
        /** \param sScriptName The name of the script
        *   \param bDefined    'true' if the script is defined
        */
        void notify_script_defined(const std::string& sScriptName, bool bDefined);

        /// Calls a script.
        /** \param sScriptName The name of the script
        *   \param pEvent      Stores scripts arguments
        */
        virtual void on(const std::string& sScriptName, event* pEvent = nullptr);

        /// Calls the on_event script.
        /** \param mEvent The Event that occured
        */
        virtual void on_event(const event& mEvent);

        /// Tells this frame to react to every event in the game.
        void register_all_events();

        /// Tells this frame to react to a certain event.
        /** \param sEventName The name of the event
        */
        void register_event(const std::string& sEventName);

        /// Tells this frame to react to mouse drag.
        /** \param lButtonList The list of mouse button allowed
        */
        void register_for_drag(const std::vector<std::string>& lButtonList);

        /// Sets if this frame is clamped to screen.
        /** \param bIsClampedToScreen 'true' if this frame is clamped to screen
        *   \note If 'true', the frame can't go out of the screen.
        */
        void set_clamped_to_screen(bool bIsClampedToScreen);

        /// Sets this frame's strata.
        /** \param mStrata The new strata
        */
        void set_frame_strata(frame_strata mStrata);

        /// Sets this frame's strata.
        /** \param sStrata The new strata
        */
        void set_frame_strata(const std::string& sStrata);

        /// Sets this frames' backdrop.
        /** \param pBackdrop The new backdrop
        */
        void set_backdrop(utils::refptr<backdrop> pBackdrop);

        /// Sets this frame's absolute hit rect insets.
        /** \param iLeft   Offset from the left border
        *   \param iRight  Offset from the right border
        *   \param iTop    Offset from the top border
        *   \param iBottom Offset from the bottom border
        *   \note This is the zone on which you can click.
        */
        void set_abs_hit_rect_insets(int iLeft, int iRight, int iTop, int iBottom);

        /// Sets this frame's absolute hit rect insets.
        /** \param lInsets Offsets
        *   \note This is the zone on which you can click.
        */
        void set_abs_hit_rect_insets(const quad2i& lInsets);

        /// Sets this frame's relative hit rect insets.
        /** \param fLeft   Offset from the left border
        *   \param fRight  Offset from the right border
        *   \param fTop    Offset from the top border
        *   \param fBottom Offset from the bottom border
        *   \note This is the zone on which you can click.
        */
        void set_rel_hit_rect_insets(float fLeft, float fRight, float fTop, float fBottom);

        /// Sets this frame's relative hit rect insets.
        /** \param lInsets Offsets
        *   \note This is the zone on which you can click.
        */
        void set_rel_hit_rect_insets(const quad2f& lInsets);

        /// Sets this frame's level.
        /** \param iLevel The new level
        */
        void set_level(int iLevel);

        /// Sets this frame's maximum size.
        /** \param uiMaxWidth  The maximum width this frame can have
        *   \param uiMaxHeight The maximum height this frame can have
        */
        void set_max_resize(uint uiMaxWidth, uint uiMaxHeight);

        /// Sets this frame's maximum size.
        /** \param mMax The maximum dimensions of this frame
        */
        void set_max_resize(const vector2ui& mMax);

        /// Sets this frame's minimum size.
        /** \param uiMinWidth  The minimum width this frame can have
        *   \param uiMinHeight The minimum height this frame can have
        */
        void set_min_resize(uint uiMinWidth, uint uiMinHeight);

        /// Sets this frame's minimum size.
        /** \param mMin Minimum dimensions of this frame
        */
        void set_min_resize(const vector2ui& mMin);

        /// Sets this frame's maximum height.
        /** \param uiMaxHeight The maximum height this frame can have
        */
        void set_max_height(uint uiMaxHeight);

        /// Sets this frame's maximum width.
        /** \param uiMaxWidth  The maximum width this frame can have
        */
        void set_max_width(uint uiMaxWidth);

        /// Sets this frame's minimum height.
        /** \param uiMinHeight The minimum height this frame can have
        */
        void set_min_height(uint uiMinHeight);

        /// Sets this frame's minimum width.
        /** \param uiMinWidth  The minimum width this frame can have
        */
        void set_min_width(uint uiMinWidth);

        /// Sets if this frame can be moved by the user.
        /** \param bIsMovable 'true' to allow the user to move this frame
        */
        void set_movable(bool bIsMovable);

        /// Changes this widget's parent.
        /** \param pParent The new parent
        *   \note Default is nullptr.<br>
        *         Overrides uiobject's implementation.
        */
        void set_parent(uiobject* pParent);

        /// Sets if this frame can be resized by the user.
        /** \param bIsResizable 'true' to allow the user to resize this frame
        */
        void set_resizable(bool bIsResizable);

        /// Sets this frame's scale.
        /** \param fScale The new scale
        */
        void set_scale(float fScale);

        /// Sets if this frame is at top level.
        /** \param bIsTopLevel 'true' to put the frame at top level
        */
        void set_top_level(bool bIsTopLevel);

        /// Increases this frame's level so it's the highest of the strata.
        /** \note All its children are raised of the same ammount.
        *   \note Only works for top level frames.
        */
        void raise();

        /// Sets if this frame has been moved by the user.
        /** \param bIsUserPlaced 'true' if this frame has been moved by the user
        */
        void set_user_placed(bool bIsUserPlaced);

        /// Starts moving this frame with the mouse.
        void start_moving();

        /// ends moving this frame.
        void stop_moving();

        /// Starts resizing this frame with the mouse.
        /** \param mPoint The corner to move
        */
        void start_sizing(const anchor_point& mPoint);

        /// ends resizing this frame.
        void stop_sizing();

        /// shows this widget.
        /** \note Its parent must be shown for it to appear on
        *         the screen.
        */
        virtual void show();

        /// hides this widget.
        /** \note All its children won't be visible on the screen
        *         anymore, even if they are still marked as shown.
        */
        virtual void hide();

        /// shows/hides this widget.
        /** \param bIsShown 'true' if you want to show this widget
        *   \note See show() and hide() for more infos.
        *   \note Contrary to show() and hide(), this function doesn't
        *         trigger any event ("OnShow" or "OnHide"). It should
        *         only be used to set the initial state of the widget.
        */
        virtual void set_shown(bool bIsShown);

        /// Flags this object as "manually rendered".
        /** \param bManuallyRendered 'true' to flag it as manually rendered
        *   \param pRenderer         The uiobject that will take care of
        *                            rendering this widget
        *   \note Manually rendered objects are not automatically rendered
        *         by their parent (for layered_regions) or the manager
        *         (for frames). They also don't receive automatic input.
        *   \note This function propagates the manually rendered flag to
        *         this frame's children.
        */
        virtual void set_manually_rendered(bool bManuallyRendered, uiobject* pRenderer = nullptr);

        /// Changes this widget's absolute dimensions (in pixels).
        /** \param uiAbsWidth  The new width
        *   \param uiAbsHeight The new height
        */
        virtual void set_abs_dimensions(uint uiAbsWidth, uint uiAbsHeight);

        /// Changes this widget's absolute width (in pixels).
        /** \param uiAbsWidth The new width
        */
        virtual void set_abs_width(uint uiAbsWidth);

        /// Changes this widget's absolute height (in pixels).
        /** \param uiAbsHeight The new height
        */
        virtual void set_abs_height(uint uiAbsHeight);

        /// Tells this frame it is being overed by the mouse.
        /** \param bMouseInFrame 'true' if the mouse is above this frame
        *   \param iX            The horizontal mouse coordinate
        *   \param iY            The vertical mouse coordinate
        *   \note Always use the mouse position set by this function and
        *         not the one returned by the InputManager, because there
        *         can be an offset to apply (for example with ScrollFrame).
        */
        virtual void notify_mouse_in_frame(bool bMouseInFrame, int iX, int iY);

        /// Tells this frame that at least one of its children has modified its strata or level.
        /** \param pChild The child that has changed its strata (can also be a child of this child)
        *   \note If this frame has no parent, it calls manager::fire_build_strata_list(). Else it
        *         notifies its parent.
        */
        virtual void notify_child_strata_changed(frame* pChild);

        /// Notifies the renderer of this widget that it needs to be redrawn.
        /** \note Automatically called by any shape changing function.
        */
        virtual void notify_renderer_need_redraw() const;

        /// Notifies this widget that it has been fully loaded.
        /** \note Calls the "OnLoad" script.
        */
        virtual void notify_loaded();

        /// Tells this frame to rebuilt the layer list.
        /** \note Automatically called by add_region(), remove_region(), and
        *         layered_region::set_draw_layer().
        */
        void fire_build_layer_list();

        /// Tells the frame not to react to all events.
        void unregister_all_events();

        /// Tells the frame not to react to a certain event.
        /** \param sEventName The name of the event
        */
        void unregister_event(const std::string& sEventName);

        /// Sets the addon this frame belongs to.
        /** \param pAddOn The addon this frame belongs to
        */
        void set_addon(addon* pAddOn);

        /// Returns this frame's addon.
        /** \return This frame's addon
        *   \note Returns "nullptr" if the frame has been created
        *         by Lua code and wasn't assigned a parent.
        */
        addon* get_addon() const;

        /// Removes all anchors that point to this widget and all other kind of links.
        /** \return The list of all widgets that have been cleared
        *   \note Also clears children objects (see frame::clear_links()).
        *   \note Must be called before deleting the widget, except when closing the whole UI.
        */
        virtual std::vector<uiobject*> clear_links();

        /// Creates the associated Lua glue.
        virtual void create_glue();

        /// Parses data from an xml::block.
        /** \param pBlock The frame's xml::block
        */
        virtual void parse_block(xml::block* pBlock);

        #ifndef NO_CPP11_CONSTEXPR
        static constexpr const char* CLASS_NAME = "Frame";
        #else
        static const char* CLASS_NAME;
        #endif

    protected :

        // XML parsing
        virtual void parse_attributes_(xml::block* pBlock);
        virtual void parse_resize_bounds_block_(xml::block* pBlock);
        virtual void parse_title_region_block_(xml::block* pBlock);
        virtual void parse_backdrop_block_(xml::block* pBlock);
        virtual void parse_hit_rect_insets_block_(xml::block* pBlock);
        virtual void parse_layers_block_(xml::block* pBlock);
        virtual void parse_frames_block_(xml::block* pBlock);
        virtual void parse_scripts_block_(xml::block* pBlock);

        virtual void notify_visible_(bool bTriggerEvents = true);
        virtual void notify_invisible_(bool bTriggerEvents = true);
        virtual void notify_strata_changed_();

        virtual void notify_top_level_parent_(bool bTopLevel, frame* pParent);

        void add_level_(int iAmount);

        virtual void update_borders_() const;

        struct script_info
        {
            std::string sFile;
            uint        uiLineNbr;
        };

        std::map<uint, frame*>             lChildList_;
        std::map<uint, layered_region*>    lRegionList_;
        std::map<layer_type, layer>        lLayerList_;
        std::map<std::string, std::string> lDefinedScriptList_;
        std::map<std::string, script_info> lXMLScriptInfoList_;
        std::vector<std::string>           lQueuedEventList_;
        std::set<std::string>              lRegEventList_;
        std::set<std::string>              lRegDragList_;

        std::map<std::string, handler> lDefinedHandlerList_;

        addon* pAddOn_;

        int iLevel_;

        frame_strata mStrata_;
        bool         bIsTopLevel_;
        frame*       pTopLevelParent_;

        utils::refptr<backdrop> pBackdrop_;

        bool bHasAllEventsRegistred_;

        bool bIsKeyboardEnabled_;
        bool bIsMouseEnabled_;
        bool bAllowWorldInput_;
        bool bIsMouseWheelEnabled_;
        bool bIsMovable_;
        bool bIsClampedToScreen_;
        bool bIsResizable_;
        bool bIsUserPlaced_;

        bool bBuildLayerList_;

        quad2i lAbsHitRectInsetList_;
        quad2f lRelHitRectInsetList_;

        uint uiMinWidth_;
        uint uiMaxWidth_;
        uint uiMinHeight_;
        uint uiMaxHeight_;

        float fScale_;

        bool bMouseInFrame_;
        bool bMouseInTitleRegion_;
        int  iMousePosX_, iMousePosY_;

        region* pTitleRegion_;

        frame* pParentFrame_;

        std::vector<std::string> lMouseButtonList_;
        bool                     bMouseDragged_;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_frame : public lua_uiobject
    {
    public :

        explicit lua_frame(lua_State* pLua);
        virtual ~lua_frame() {}

        int _create_font_string(lua_State*);
        int _create_texture(lua_State*);
        int _create_title_region(lua_State*);
        int _disable_draw_layer(lua_State*);
        int _enable_draw_layer(lua_State*);
        int _enable_keyboard(lua_State*);
        int _enable_mouse(lua_State*);
        int _enable_mouse_wheel(lua_State*);
        int _get_backdrop(lua_State*);
        int _get_backdrop_border_color(lua_State*);
        int _get_backdrop_color(lua_State*);
        int _get_children(lua_State*);
        int _get_effective_alpha(lua_State*);
        int _get_effective_scale(lua_State*);
        int _get_frame_level(lua_State*);
        int _get_frame_strata(lua_State*);
        int _get_frame_type(lua_State*);
        int _get_hit_rect_insets(lua_State*);
        int _get_id(lua_State*);
        int _get_max_resize(lua_State*);
        int _get_min_resize(lua_State*);
        int _set_max_width(lua_State*);
        int _set_max_height(lua_State*);
        int _set_min_width(lua_State*);
        int _set_min_height(lua_State*);
        int _get_num_children(lua_State*);
        int _get_num_regions(lua_State*);
        int _get_scale(lua_State*);
        int _get_script(lua_State*);
        int _get_title_region(lua_State*);
        int _has_script(lua_State*);
        int _is_clamped_to_screen(lua_State*);
        int _is_frame_type(lua_State*);
        int _is_keyboard_enabled(lua_State*);
        int _is_mouse_enabled(lua_State*);
        int _is_mouse_wheel_enabled(lua_State*);
        int _is_movable(lua_State*);
        int _is_resizable(lua_State*);
        int _is_top_level(lua_State*);
        int _is_user_placed(lua_State*);
        int _on(lua_State*);
        int _raise(lua_State*);
        int _register_all_events(lua_State*);
        int _register_event(lua_State*);
        int _register_for_drag(lua_State*);
        int _set_backdrop(lua_State*);
        int _set_backdrop_border_color(lua_State*);
        int _set_backdrop_color(lua_State*);
        int _set_clamped_to_screen(lua_State*);
        int _set_frame_level(lua_State*);
        int _set_frame_strata(lua_State*);
        int _set_hit_rect_insets(lua_State*);
        int _set_max_resize(lua_State*);
        int _set_min_resize(lua_State*);
        int _set_movable(lua_State*);
        int _set_resizable(lua_State*);
        int _set_scale(lua_State*);
        int _set_script(lua_State*);
        int _set_top_level(lua_State*);
        int _set_user_placed(lua_State*);
        int _start_moving(lua_State*);
        int _start_sizing(lua_State*);
        int _stop_moving_or_sizing(lua_State*);
        int _unregister_all_events(lua_State*);
        int _unregister_event(lua_State*);

        static const char className[];
        static const char* classList[];
        static lua::glue_list<lua_frame> methods[];

    protected :

        frame* pFrameParent_;
    };

    /** \endcond
    */
}

#endif
