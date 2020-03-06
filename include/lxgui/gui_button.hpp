#ifndef GUI_BUTTON_HPP
#define GUI_BUTTON_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_frame.hpp"

namespace gui
{
    class texture;
    class font_string;

    /// A simple button.
    /** This class can handle three different states :
    *   "normal", "pushed" and "disabled". You can provide a
    *   different texture for each of these states, and
    *   two different fontstrings for "normal" and "disabled".<br>
    *   In addition, you can provide another texture/fontstring
    *   for the "highlight" state (when the mouse is over the
    *   button widget).<br>
    *   Note that there is no fontstring for the "pushed" state :
    *   in this case, the "normal" font is rendered with a slight
    *   offset that you'll have to define.
    */
    class button : public frame
    {
    public :

        enum state
        {
            STATE_UP,
            STATE_DOWN,
            STATE_DISABLED
        };

        /// Constructor.
        explicit button(manager* pManager);

        /// Destructor.
        virtual ~button();

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        virtual std::string serialize(const std::string& sTab) const;

        /// Creates the associated Lua glue.
        virtual void create_glue();

        /// Returns 'true' if this Button can use a script.
        /** \param sScriptName The name of the script
        *   \note This method can be overriden if needed.
        */
        virtual bool can_use_script(const std::string& sScriptName) const;

        /// Calls a script.
        /** \param sScriptName The name of the script
        *   \param pEvent      Stores scripts arguments
        */
        virtual void on(const std::string& sScriptName, event* pEvent = nullptr);

        /// Calls the on_event script.
        /** \param mEvent The Event that occured
        */
        virtual void on_event(const event& mEvent);

        /// Copies an uiobject's parameters into this Button (inheritance).
        /** \param pObj The uiobject to copy
        */
        virtual void copy_from(uiobject* pObj);

        /// Sets this button's text.
        /** \param sText The new text
        */
        virtual void set_text(const std::string& sText);

        /// Returns this button's text.
        /** \return This button's text
        */
        const std::string& get_text() const;

        /// Returns this button's normal texture.
        /** \return This button's normal texture
        */
        texture* get_normal_texture();

        /// Returns this button's pushed texture.
        /** \return This button's pushed texture
        */
        texture* get_pushed_texture();

        /// Returns this button's disabled texture.
        /** \return This button's disabled texture
        */
        texture* get_disabled_texture();

        /// Returns this button's highlight texture.
        /** \return This button's highlight texture
        */
        texture* get_highlight_texture();

        /// Returns this button's normal text.
        /** \return This button's normal text
        */
        font_string* get_normal_text();

        /// Returns this button's highlight text.
        /** \return This button's highlight text
        */
        font_string* get_highlight_text();

        /// Returns this button's disabled text.
        /** \return This button's disabled text
        */
        font_string* get_disabled_text();

        /// Returns the currently displayed text object.
        /** \return The currently displayed text object
        */
        font_string* get_CurrentFontString();

        /// Sets this button's normal texture.
        /** \param pTexture The new texture
        */
        void set_normal_texture(texture* pTexture);

        /// Sets this button's pushed texture.
        /** \param pTexture The new texture
        */
        void set_pushed_texture(texture* pTexture);

        /// Sets this button's disabled texture.
        /** \param pTexture The new texture
        */
        void set_disabled_texture(texture* pTexture);

        /// Sets this button's highlight texture.
        /** \param pTexture The new texture
        */
        void set_highlight_texture(texture* pTexture);

        /// Sets this button's normal text.
        /** \param pFont The new text object
        */
        void set_normal_text(font_string* pFont);

        /// Sets this button's highlight text.
        /** \param pFont The new text object
        */
        void set_highlight_text(font_string* pFont);

        /// Sets this button's disabled text.
        /** \param pFont The new text object
        */
        void set_disabled_text(font_string* pFont);

        /// Disables this Button.
        /** \note A disabled button doesn't receive any input.
        */
        virtual void disable();

        /// Enables this Button.
        virtual void enable();

        /// Checks if this Button is enabled.
        /** \return 'true' if this Button is enabled
        */
        bool is_enabled() const;

        /// Pushed this Button.
        /** \note This function only has a visual impact :
        *         the OnClick() handler is not called.
        */
        virtual void push();

        /// Releases this Button.
        /** \note This function only has a visual impact :
        *         the OnClick() handler is not called.
        */
        virtual void release();

        /// Highlights this Button.
        /** \note The Button will be highlighted even if the
        *         mouse is not over it. It will stop when the
        *         mouse leaves it.
        */
        virtual void highlight();

        /// Unlights this Button.
        /** \note The Button will be unlighted even if the
        *         mouse is over it. It will highlight again
        *         when the mouse leaves then enters its region.
        */
        virtual void unlight();

        /// Returns this button's state.
        /** \return This button's state (see ButtonState)
        */
        state get_button_state() const;

        /// Locks this button's highlighting.
        /** \note The button will always be highlighted
        *         until you call unlock_highlight().
        */
        void lock_highlight();

        /// Unlocks this button's highlighting.
        void unlock_highlight();

        /// Sets this button's pushed text offset.
        /** \param lOffset The pushed text offset
        */
        virtual void set_pushed_text_offset(const vector2i& lOffset);

        /// Returns this button's pushed text offset.
        /** \return This button's pushed text offset
        */
        const vector2i& get_pushed_text_offset() const;

        /// Parses data from an xml::block.
        /** \param pBlock The button's xml::block
        */
        virtual void parse_block(xml::block* pBlock);

        /// Registers this widget to the provided lua::state
        static void register_glue(utils::wptr<lua::state> pLua);

        #ifndef NO_CPP11_CONSTEXPR
        static constexpr const char* CLASS_NAME = "Button";
        #else
        static const char* CLASS_NAME;
        #endif

    protected :

        texture*     create_normal_texture_();
        texture*     create_pushed_texture_();
        texture*     create_disabled_texture_();
        texture*     create_highlight_texture_();
        font_string* create_normal_text_();
        font_string* create_highlight_text_();
        font_string* create_disabled_text_();

        state     mState_;
        bool      bHighlighted_;
        bool      bLockHighlight_;

        std::string sText_;

        texture* pNormalTexture_;
        texture* pPushedTexture_;
        texture* pDisabledTexture_;
        texture* pHighlightTexture_;

        font_string* pNormalText_;
        font_string* pHighlightText_;
        font_string* pDisabledText_;
        font_string* pCurrentFontString_;

        vector2i mPushedTextOffset_;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_button : public lua_frame
    {
    public :

        explicit lua_button(lua_State* pLua);
        virtual ~lua_button() {}

        // Glues
        int _click(lua_State*);
        int _disable(lua_State*);
        int _enable(lua_State*);
        int _get_button_state(lua_State*);
        int _get_disabled_font_object(lua_State*);
        int _get_disabled_text_color(lua_State*);
        int _get_disabled_texture(lua_State*);
        int _get_highlight_font_object(lua_State*);
        int _get_highlight_text_color(lua_State*);
        int _get_highlight_texture(lua_State*);
        int _get_normal_font_object(lua_State*);
        int _get_normal_texture(lua_State*);
        int _get_pushed_text_offset(lua_State*);
        int _get_pushed_texture(lua_State*);
        int _get_text(lua_State*);
        int _get_text_height(lua_State*);
        int _get_text_width(lua_State*);
        int _is_enabled(lua_State*);
        int _lock_highlight(lua_State*);
        int _set_button_state(lua_State*);
        int _set_disabled_font_object(lua_State*);
        int _set_disabled_text_color(lua_State*);
        int _set_disabled_texture(lua_State*);
        int _set_highlight_font_object(lua_State*);
        int _set_highlight_text_color(lua_State*);
        int _set_highlight_texture(lua_State*);
        int _set_normal_font_object(lua_State*);
        int _set_normal_text_color(lua_State*);
        int _set_normal_texture(lua_State*);
        int _set_pushed_text_offset(lua_State*);
        int _set_pushed_texture(lua_State*);
        int _set_text(lua_State*);

        int _unlock_highlight(lua_State*);

        static const char className[];
        static const char* classList[];
        static lua::glue_list<lua_button> methods[];

    protected :

        button* pButtonParent_;
    };

    /** \endcond
    */
}

#endif
