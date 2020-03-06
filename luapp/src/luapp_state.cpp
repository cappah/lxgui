#include "lxgui/luapp_state.hpp"
#include "lxgui/luapp_glues.hpp"
#include "lxgui/luapp_exception.hpp"
#include "lxgui/luapp_var.hpp"
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <iostream>
#include <deque>

namespace lua
{
const luaL_Reg lualibs[] = {
    {"",              luaopen_base},
    {LUA_LOADLIBNAME, luaopen_package},
    {LUA_TABLIBNAME,  luaopen_table},
    {LUA_IOLIBNAME,   luaopen_io},
    {LUA_OSLIBNAME,   luaopen_os},
    {LUA_STRLIBNAME,  luaopen_string},
    {LUA_MATHLIBNAME, luaopen_math},
    {LUA_DBLIBNAME,   luaopen_debug},
    {nullptr,         nullptr}
};

std::map<lua_State*, state*> state::lLuaStateMap_;

void open_libs(lua_State* pLua_)
{
    for (const luaL_Reg* lib = lualibs; lib->func != nullptr; ++lib)
    {
        lua_pushcfunction(pLua_, lib->func);
        lua_pushstring(pLua_, lib->name);
        lua_call(pLua_, 1, 0);
    }
}

int  l_treat_error(lua_State*);
void default_print_function(const std::string&);

state::state()
{
    pErrorFunction_ = &l_treat_error;
    pPrintFunction_ = &default_print_function;

    pLua_ = luaL_newstate();
    if (!pLua_)
        throw lua::exception("state", "Error while initializing Lua.");

    open_libs(pLua_);

    reg("SendString",  glue_send_string);
    reg("EmptyString", glue_empty_string);
    reg("ConcTable",   glue_table_to_string);

    lLuaStateMap_[pLua_] = this;
}

state::~state()
{
    if (pLua_)
        lua_close(pLua_);

    lLuaStateMap_.erase(pLua_);
}

state* state::get_state(lua_State* pLua)
{
    std::map<lua_State*, state*>::iterator iter = lLuaStateMap_.find(pLua);
    if (iter != lLuaStateMap_.end())
        return iter->second;
    else
        return nullptr;
}

lua_State* state::get_state()
{
    return pLua_;
}

std::string state::table_to_string(const std::string& sTable)
{
    /* [#] This function converts a Lua table into a formated string. It is used
    *  to save the content of the table in the SavedVariables.
    */
    std::string s = "tbl = \"" + sTable + "\";\n"
              "temp = \"\";\n"
              "for k, v in pairs (" + sTable + ") do\n"
                  "local s, t;\n"
                  "if (type(k) == \"number\") then\ns = k;\nend\n"
                  "if (type(k) == \"string\") then\ns = \"\\\"\"..k..\"\\\"\";\nend\n"
                  "if (type(v) == \"number\") then\nt = v;\nend\n"
                  "if (type(v) == \"string\") then\nt = \"\\\"\"..v..\"\\\"\";\nend\n"
                  "if (type(v) == \"boolean\") then\nif v then\nt = \"'true'\";\nelse\nt = \"'false'\";\nend\nend\n"
                  "if (type(v) == \"table\") then\n"
                      "t = \"'table'\";\nSendString(s..\" \"..t..\" \");\nConcTable(temp, tbl..\"[\"..s..\"]\");\n"
                  "else\n"
                      "SendString(s..\" \"..t..\" \");\n"
                  "end\n"
              "end\n"
              "SendString(\"'end' \");\n";

    luaL_dostring(pLua_, s.c_str());

    return sComString;
}

void state::copy_table(state& mLua, const std::string& sSrcName, const std::string& sDestName)
{
    std::string sNewName;
    if (sDestName == "") sNewName = sSrcName;
    else                 sNewName = sDestName;

    sComString = "";
    try
    {
        mLua.do_string("str = \"\";\nstr = ConcTable(str, \"" + sSrcName + "\");\n");
    }
    catch (exception& e)
    {
        throw lua::exception("state", "copy_table : "+e.get_description());
    }

    std::string s = sComString;

    if (s != "")
    {
        std::string sTab = "    ";
        std::string sTable;
        sTable = sNewName + " = {\n";
        uint uiTableIndent = 1u;
        bool bTableEnded = false;
        uint uiLineNbr = 0u;
        while (!bTableEnded)
        {
            if (uiLineNbr > 1000u)
                break;

            if (uiTableIndent == 0)
            {
                bTableEnded = true;
            }
            else
            {
                size_t i = s.find(" ");
                std::string sKey = s.substr(0, i);
                ++i;
                s.erase(0, i);
                if (sKey == "'end'")
                {
                    --uiTableIndent;
                    sTab = sTab.substr(0, sTab.size() - 4u);
                    if (uiTableIndent == 0)
                        sTable += "}\n";
                    else
                        sTable += sTab + "},\n";
                }
                else
                {
                    sKey = "[" + sKey + "]";

                    i = s.find(" ");
                    std::string sValue = s.substr(0, i);
                    ++i;
                    s.erase(0, i);

                    int iType;
                    if (sValue == "'table'")
                        iType = TYPE_TABLE;
                    else
                    {
                        iType = TYPE_NUMBER;
                        if (sValue.find("\"") != std::string::npos)
                        {
                            iType = TYPE_STRING;
                        }
                        else
                        {
                            if (sValue.find("'") != std::string::npos)
                            {
                                iType = TYPE_BOOLEAN;
                                utils::trim(sValue, '\'');
                            }
                        }
                    }

                    if (iType == TYPE_NUMBER)
                    {
                        sTable += sTab + sKey + " = " + sValue + ";\n";
                    }
                    else if (iType == TYPE_NIL)
                    {
                        sTable += sTab + sKey + " = nil;\n";
                    }
                    else if (iType == TYPE_BOOLEAN)
                    {
                        sTable += sTab + sKey + " = " + sValue + ";\n";
                    }
                    else if (iType == TYPE_STRING)
                    {
                        sTable += sTab + sKey + " = " + sValue + ";\n";
                    }
                    else if (iType == TYPE_TABLE)
                    {
                        sTable += sTab + sKey + " = {\n";
                        sTab += "    ";
                    }
                }
            }
        }

        mLua.do_string(sTable);
    }
    else
    {
        mLua.new_table();
        mLua.set_global(sNewName);
    }
}

int l_treat_error(lua_State* pLua)
{
    if (!lua_isstring(pLua, -1))
        return 0;

    lua_Debug d;

    lua_getstack(pLua, 1, &d);
    lua_getinfo(pLua, "Sl", &d);

    std::string sError = std::string(d.short_src) + ":" + utils::to_string(int(d.currentline)) + ": " + std::string(lua_tostring(pLua, -1));
    lua_pushstring(pLua, sError.c_str());

    return 1;
}

void default_print_function(const std::string& s)
{
    std::cerr << s << std::endl;
}

void state::do_file(const std::string& sFile)
{
    if (utils::file_exists(sFile))
    {
        lua_pushcfunction(pLua_, pErrorFunction_);
        uint uiFuncPos = get_top();

        if (luaL_loadfile(pLua_, sFile.c_str()) != 0)
        {
            if (lua_isstring(pLua_, -1))
            {
                std::string sError = lua_tostring(pLua_, -1);
                lua_pop(pLua_, 1);
                lua_remove(pLua_, uiFuncPos);
                throw lua::exception(sError);
            }
            else
            {
                lua_remove(pLua_, uiFuncPos);
                throw lua::exception("state", "Cannot load file : \""+sFile+"\"");
            }
        }

        int iError = lua_pcall(pLua_, 0, LUA_MULTRET, -2);
        if (iError != 0)
        {
            if (lua_isstring(pLua_, -1))
            {
                std::string sError = lua_tostring(pLua_, -1);
                lua_pop(pLua_, 1);
                lua_remove(pLua_, uiFuncPos);
                throw lua::exception(sError);
            }
            else
            {
                lua_pop(pLua_, 1);
                lua_remove(pLua_, uiFuncPos);
                throw lua::exception("state", "Unhandled error.");
            }
        }

        lua_remove(pLua_, uiFuncPos);
    }
    else
        throw lua::exception("state", "do_file : cannot open \""+sFile+"\".");
}

void state::do_string(const std::string& sStr)
{
    lua_pushcfunction(pLua_, pErrorFunction_);
    uint uiFuncPos = get_top();

    if (luaL_loadstring(pLua_, sStr.c_str()) != 0)
    {
        if (lua_isstring(pLua_, -1))
        {
            std::string sError = lua_tostring(pLua_, -1);
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception(sError);
        }
        else
        {
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception("state", "Unhandled error.");
        }
    }

    int iError = lua_pcall(pLua_, 0, LUA_MULTRET, -2);
    if (iError != 0)
    {
        if (lua_isstring(pLua_, -1))
        {
            std::string sError = lua_tostring(pLua_, -1);
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception(sError);
        }
        else
        {
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception("state", "Unhandled error.");
        }
    }

    lua_remove(pLua_, uiFuncPos);
}

void state::call_function(const std::string& sFunctionName)
{
    lua_pushcfunction(pLua_, pErrorFunction_);
    uint uiFuncPos = get_top();

    get_global(sFunctionName);

    if (lua_isnil(pLua_, -1))
    {
        lua_settop(pLua_, uiFuncPos-1);
        throw lua::exception("state", "\""+sFunctionName+"\" doesn't exist.");
    }

    if (!lua_isfunction(pLua_, -1))
    {
        lua::exception mExcept("state", "\""+sFunctionName+"\" is not a function ("+
            get_type_name(get_type())+" : "+get_value().to_string()+")."
        );
        lua_settop(pLua_, uiFuncPos-1);
        throw mExcept;
    }

    bool bObject = false;
    std::string::const_reverse_iterator iter;
    for (iter = sFunctionName.rbegin(); iter != sFunctionName.rend(); ++iter)
    {
        if (*iter == '.')
            break;

        if (*iter == ':')
        {
            std::string sObject = sFunctionName.substr(0, sFunctionName.size() - 1 - (iter - sFunctionName.rbegin()));
            get_global(sObject);
            if (!lua_isnil(pLua_, -1))
                bObject = true;
            else
                lua_pop(pLua_, 1);
        }
    }

    int iError;
    if (bObject)
        iError = lua_pcall(pLua_, 1, LUA_MULTRET, -3);
    else
        iError = lua_pcall(pLua_, 0, LUA_MULTRET, -2);

    if (iError != 0)
    {
        if (lua_isstring(pLua_, -1))
        {
            lua::exception mExcept(lua_tostring(pLua_, -1));
            lua_settop(pLua_, uiFuncPos-1);
            throw mExcept;
        }
        else
        {
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception("state", "Unhandled error.");
        }
    }

    lua_remove(pLua_, uiFuncPos);
}

void state::call_function(const std::string& sFunctionName, const std::vector<var>& lArgumentStack)
{
    lua_pushcfunction(pLua_, pErrorFunction_);
    uint uiFuncPos = get_top();

    get_global(sFunctionName);

    if (lua_isnil(pLua_, -1))
    {
        lua_settop(pLua_, uiFuncPos-1);
        throw lua::exception("state", "\""+sFunctionName+"\" doesn't exist.");
    }

    if (lua_isfunction(pLua_, -1))
    {
        lua_settop(pLua_, uiFuncPos-1);
        throw lua::exception("state", "\""+sFunctionName+"\" is not a function ("+get_type_name(get_type())+" : "+get_value().to_string()+")");
    }

    std::vector<var>::const_iterator iterArg;
    foreach (iterArg, lArgumentStack)
        push(*iterArg);

    bool bObject = false;
    std::string::const_reverse_iterator iter;
    for (iter = sFunctionName.rbegin(); iter != sFunctionName.rend(); ++iter)
    {
        if (*iter == '.')
            break;

        if (*iter == ':')
        {
            std::string sObject = sFunctionName.substr(0, sFunctionName.size() - 1 - (iter - sFunctionName.rbegin()));
            get_global(sObject);
            if (!lua_isnil(pLua_, -1))
                bObject = true;
            else
                lua_pop(pLua_, 1);
        }
    }

    int iError;
    if (bObject)
        iError = lua_pcall(pLua_, lArgumentStack.size() + 1, LUA_MULTRET, -3-lArgumentStack.size());
    else
        iError = lua_pcall(pLua_, lArgumentStack.size(), LUA_MULTRET, -2-lArgumentStack.size());

    if (iError != 0)
    {
        if (lua_isstring(pLua_, -1))
        {
            std::string sError = lua_tostring(pLua_, -1);
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception(sError);
        }
        else
        {
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception("state", "Unhandled error.");
        }
    }

    lua_remove(pLua_, uiFuncPos);
}

void state::reg(const std::string& sFunctionName, c_function mFunction)
{
    lua_register(pLua_, sFunctionName.c_str(), mFunction);
}

std::string state::format_error(const std::string& sError)
{
    push_string(sError);
    (*pErrorFunction_)(pLua_);
    std::string sResult = get_string();
    pop(2);

    return sResult;
}

void state::print_error(const std::string& sError)
{
    (*pPrintFunction_)(format_error(sError));
}

void state::set_lua_error_function(c_function pFunc)
{
    if (pFunc != 0)
        pErrorFunction_ = pFunc;
    else
        pErrorFunction_ = &l_treat_error;
}

c_function state::get_lua_error_function() const
{
    return pErrorFunction_;
}

void state::set_print_error_function(print_function pFunc)
{
    if (pFunc != 0)
        pPrintFunction_ = pFunc;
    else
        pPrintFunction_ = &default_print_function;
}

bool state::is_serializable(int iIndex)
{
    int iAbsoluteIndex = iIndex >= 0 ? iIndex : int(get_top()+1) + iIndex;
    if (lua_getmetatable(pLua_, iAbsoluteIndex))
    {
        pop();
        get_field("serialize", iAbsoluteIndex);
        if (get_type() == TYPE_FUNCTION)
        {
            pop();
            return true;
        }
        pop();
    }

    type mType = get_type(iIndex);
    return (mType == TYPE_BOOLEAN) || (mType == TYPE_NUMBER) ||
           (mType == TYPE_STRING)  || (mType == TYPE_TABLE) || (mType == TYPE_NIL);
}

std::string state::serialize_global(const std::string& sName)
{
    get_global(sName);

    std::string sContent;
    if (is_serializable())
        sContent = sName+" = "+serialize()+";";

    pop();

    return sContent;
}

std::string state::serialize(const std::string& sTab, int iIndex)
{
    int iAbsoluteIndex = iIndex >= 0 ? iIndex : int(get_top()+1) + iIndex;
    std::string sResult;

    if (lua_getmetatable(pLua_, iAbsoluteIndex))
    {
        pop();
        get_field("serialize", iAbsoluteIndex);
        if (get_type() == TYPE_FUNCTION)
        {
            push_value(iAbsoluteIndex);
            push_string(sTab);
            int iError = lua_pcall(pLua_, 2, 1, 0);
            if (iError == 0)
            {
                sResult += get_string();
                pop();
                return sResult;
            }
        }
        else
            pop();
    }

    type mType = get_type(iIndex);
    switch (mType)
    {
        case TYPE_NIL :
            sResult += "nil";
            break;

        case TYPE_BOOLEAN :
            sResult += utils::to_string(get_bool(iIndex));
            break;

        case TYPE_NUMBER :
            sResult += utils::to_string(get_number(iIndex));
            break;

        case TYPE_STRING :
            sResult += "\""+get_string(iIndex)+"\"";
            break;

        case TYPE_TABLE :
        {
            sResult += "{";

            std::string sContent = "\n";
            push_value(iIndex);
            for (push_nil(); next(); pop())
            {
                if (is_serializable())
                {
                    sContent += sTab + "    [" + serialize(sTab + "    ", -2) + "] = "
                        + serialize(sTab + "    ", -1) + ",\n";
                }
            }
            pop();

            if (sContent != "\n")
                sResult + sContent + sTab;

            sResult += "}";
            break;
        }

        case TYPE_NONE :
        case TYPE_FUNCTION :
        case TYPE_THREAD :
        default : break;
    }

    return sResult;
}

void state::push_number(const double& dValue)
{
    lua_pushnumber(pLua_, dValue);
}

void state::push_bool(bool bValue)
{
    lua_pushboolean(pLua_, bValue);
}

void state::push_string(const std::string& sValue)
{
    lua_pushstring(pLua_, sValue.c_str());
}

void state::push_nil(uint uiNumber)
{
    for (uint ui = 0u; ui < uiNumber; ++ui)
        lua_pushnil(pLua_);
}

void state::push(const var& vValue)
{
    const var_type& mType = vValue.get_type();
    if      (mType == var::VALUE_INT)    push_number(vValue.get<int>());
    else if (mType == var::VALUE_UINT)   push_number(vValue.get<uint>());
    else if (mType == var::VALUE_FLOAT)  push_number(vValue.get<float>());
    else if (mType == var::VALUE_DOUBLE) push_number(vValue.get<double>());
    else if (mType == var::VALUE_STRING) push_string(vValue.get<std::string>());
    else if (mType == var::VALUE_BOOL)   push_bool(vValue.get<bool>());
    else push_nil();
}

void state::push_value(int iIndex)
{
    lua_pushvalue(pLua_, iIndex);
}

void state::push_global(const std::string& sName)
{
    get_global(sName);
}

void state::set_global(const std::string& sName)
{
    std::deque<std::string> lDecomposedName;
    std::string sVarName;
    utils::string_vector lWords = utils::cut(sName, ":");
    utils::string_vector::iterator iter1;
    foreach (iter1, lWords)
    {
        utils::string_vector lSubWords = utils::cut(*iter1, ".");
        utils::string_vector::iterator iter2;
        foreach (iter2, lSubWords)
            lDecomposedName.push_back(*iter2);
    }

    // Start at 1 to pop the value the user has put on the stack.
    uint uiCounter = 1;

    sVarName = lDecomposedName.back();
    lDecomposedName.pop_back();

    if (lDecomposedName.size() >= 1)
    {
        lua_getglobal(pLua_, lDecomposedName.begin()->c_str());
        lDecomposedName.pop_front();
        ++uiCounter;

        if (!lua_isnil(pLua_, -1))
        {
            std::deque<std::string>::iterator iterWords;
            foreach (iterWords, lDecomposedName)
            {
                lua_getfield(pLua_, -1, iterWords->c_str());
                ++uiCounter;
                if (lua_isnil(pLua_, -1))
                {
                    pop(uiCounter);
                    return;
                }
            }
        }

        lua_pushvalue(pLua_, -(int)uiCounter);
        lua_setfield(pLua_, -2, sVarName.c_str());
        pop(uiCounter);
    }
    else
    {
        lua_setglobal(pLua_, sName.c_str());
    }
}

void state::new_table()
{
    lua_newtable(pLua_);
}

bool state::next(int iIndex)
{
    int res = lua_next(pLua_, iIndex);
    return res != 0;
}

void state::pop(uint uiNumber)
{
    lua_pop(pLua_, (int)uiNumber);
}

double state::get_number(int iIndex)
{
    return lua_tonumber(pLua_, iIndex);
}

bool state::get_bool(int iIndex)
{
    return lua_toboolean(pLua_, iIndex) != 0;
}

std::string state::get_string(int iIndex)
{
    return lua_tostring(pLua_, iIndex);
}

var state::get_value(int iIndex)
{
    int type = lua_type(pLua_, iIndex);
    switch (type)
    {
        case LUA_TBOOLEAN : return get_bool(iIndex);
        case LUA_TNUMBER : return get_number(iIndex);
        case LUA_TSTRING : return get_string(iIndex);
        default : return var();
    }
}

uint state::get_top()
{
    return lua_gettop(pLua_);
}

type state::get_type(int iIndex)
{
    int type = lua_type(pLua_, iIndex);
    switch (type)
    {
        case LUA_TBOOLEAN :       return TYPE_BOOLEAN;
        case LUA_TFUNCTION :      return TYPE_FUNCTION;
        case LUA_TLIGHTUSERDATA : return TYPE_LIGHTUSERDATA;
        case LUA_TNIL :           return TYPE_NIL;
        case LUA_TNONE :          return TYPE_NONE;
        case LUA_TNUMBER :        return TYPE_NUMBER;
        case LUA_TSTRING :        return TYPE_STRING;
        case LUA_TTABLE :         return TYPE_TABLE;
        case LUA_TTHREAD :        return TYPE_THREAD;
        case LUA_TUSERDATA :      return TYPE_USERDATA;
        default :                 return TYPE_NONE;
    }
}

std::string state::get_type_name(type mType)
{
    switch (mType)
    {
        case TYPE_BOOLEAN :       return lua_typename(pLua_, LUA_TBOOLEAN);
        case TYPE_FUNCTION :      return lua_typename(pLua_, LUA_TFUNCTION);
        case TYPE_LIGHTUSERDATA : return lua_typename(pLua_, LUA_TLIGHTUSERDATA);
        case TYPE_NIL :           return lua_typename(pLua_, LUA_TNIL);
        case TYPE_NONE :          return lua_typename(pLua_, LUA_TNONE);
        case TYPE_NUMBER :        return lua_typename(pLua_, LUA_TNUMBER);
        case TYPE_STRING :        return lua_typename(pLua_, LUA_TSTRING);
        case TYPE_TABLE :         return lua_typename(pLua_, LUA_TTABLE);
        case TYPE_THREAD :        return lua_typename(pLua_, LUA_TTHREAD);
        case TYPE_USERDATA :      return lua_typename(pLua_, LUA_TUSERDATA);
        default :                 return "";
    }
}

void state::get_global(const std::string& sName)
{
    std::deque<std::string> lDecomposedName;
    utils::string_vector lWords = utils::cut(sName, ":");
    utils::string_vector::iterator iter1;
    foreach (iter1, lWords)
    {
        utils::string_vector lSubWords = utils::cut(*iter1, ".");
        utils::string_vector::iterator iter2;
        foreach (iter2, lSubWords)
            lDecomposedName.push_back(*iter2);
    }

    lua_getglobal(pLua_, lDecomposedName.front().c_str());

    if (lDecomposedName.size() > 1)
    {
        if (lua_isnil(pLua_, -1))
            return;

        lDecomposedName.pop_front();

        std::deque<std::string>::iterator iterWords;
        foreach (iterWords, lDecomposedName)
        {
            lua_getfield(pLua_, -1, iterWords->c_str());
            lua_remove(pLua_, -2);

            if (lua_isnil(pLua_, -1))
                return;

            if (!lua_istable(pLua_, -1) && iterWords+1 != lDecomposedName.end())
            {
                lua_pop(pLua_, 1);
                lua_pushnil(pLua_);
                return;
            }
        }
    }
}

int state::get_global_int(const std::string& sName, bool bCritical, int iDefaultValue)
{
    int i;
    get_global(sName);

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");

        i = iDefaultValue;
    }
    else if (!lua_isnumber(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        print_error("\"" + sName + "\" is expected to be a number");
        i = iDefaultValue;
    }
    else
    {
        i = lua_tonumber(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return i;
}

double state::get_global_double(const std::string& sName, bool bCritical, double fDefaultValue)
{
    double f;
    get_global(sName);

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");

        f = fDefaultValue;
    }
    else if (!lua_isnumber(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        print_error("\"" + sName + "\" is expected to be a number");
        f = fDefaultValue;
    }
    else
    {
        f = lua_tonumber(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return f;
}

std::string state::get_global_string(const std::string& sName, bool bCritical, const std::string& sDefaultValue)
{
    std::string s;
    get_global(sName);

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");

        s = sDefaultValue;
    }
    else if (!lua_isstring(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        print_error("\"" + sName + "\" is expected to be a string");
        s = sDefaultValue;
    }
    else
    {
        s = lua_tostring(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return s;
}

bool state::get_global_bool(const std::string& sName, bool bCritical, bool bDefaultValue)
{
    bool b;
    get_global(sName);

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");

        b = bDefaultValue;
    }
    else if (!lua_isboolean(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        print_error("\"" + sName + "\" is expected to be a bool");
        b = bDefaultValue;
    }
    else
    {
        b = (lua_toboolean(pLua_, -1) != 0);
        lua_pop(pLua_, 1);
    }

    return b;
}

void state::get_field(const std::string& sName, int iIndex)
{
    lua_getfield(pLua_, iIndex, sName.c_str());
}

void state::get_field(int iID, int iIndex)
{
    lua_pushnumber(pLua_, iID);
    if (iIndex >= 0)
        lua_gettable(pLua_, iIndex);
    else
        lua_gettable(pLua_, iIndex - 1);
}

int state::get_field_int(const std::string& sName, bool bCritical, int iDefaultValue, bool bSetValue)
{
    int i = 0;
    lua_getfield(pLua_, -1, sName.c_str());

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");
        else if (bSetValue)
            set_field_int(sName, iDefaultValue);

        i = iDefaultValue;
    }
    else if (!lua_isnumber(pLua_, -1))
    {
        print_error("Field is expected to be a number");
        lua_pop(pLua_, 1);
        i = iDefaultValue;
    }
    else
    {
        i = lua_tonumber(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return i;
}

double state::get_field_double(const std::string& sName, bool bCritical, double fDefaultValue, bool bSetValue)
{
    double f = 0.0f;
    lua_getfield(pLua_, -1, sName.c_str());

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");
        else if (bSetValue)
            set_field_double(sName, fDefaultValue);

        f = fDefaultValue;
    }
    else if (!lua_isnumber(pLua_, -1))
    {
        print_error("Field is expected to be a number");
        lua_pop(pLua_, 1);
        f = fDefaultValue;
    }
    else
    {
        f = lua_tonumber(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return f;
}

std::string state::get_field_string(const std::string& sName, bool bCritical, const std::string& sDefaultValue, bool bSetValue)
{
    std::string s;
    lua_getfield(pLua_, -1, sName.c_str());

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");
        else if (bSetValue)
            set_field_string(sName, sDefaultValue);

        s = sDefaultValue;
    }
    else if (!lua_isstring(pLua_, -1))
    {
        print_error("Field is expected to be a string");
        lua_pop(pLua_, 1);
        s = sDefaultValue;
    }
    else
    {
        s = lua_tostring(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return s;
}

bool state::get_field_bool(const std::string& sName, bool bCritical, bool bDefaultValue, bool bSetValue)
{
    bool b = false;
    lua_getfield(pLua_, -1, sName.c_str());

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");
        else if (bSetValue)
            set_field_bool(sName, bDefaultValue);

        b = bDefaultValue;
    }
    else if (!lua_isboolean(pLua_, -1))
    {
        print_error("Field is expected to be a bool");
        lua_pop(pLua_, 1);
        b = bDefaultValue;
    }
    else
    {
        b = (lua_toboolean(pLua_, -1) != 0);
        lua_pop(pLua_, 1);
    }

    return b;
}

void state::set_field(const std::string& sName)
{
    lua_pushstring(pLua_, sName.c_str());
    lua_pushvalue(pLua_, -2);
    lua_settable(pLua_, -4);
    lua_pop(pLua_, 1);
}

void state::set_field(int iID)
{
    lua_pushnumber(pLua_, iID);
    lua_pushvalue(pLua_, -2);
    lua_settable(pLua_, -4);
    lua_pop(pLua_, 1);
}

void state::set_field_int(const std::string& sName, int iValue)
{
    lua_pushstring(pLua_, sName.c_str());
    lua_pushnumber(pLua_, iValue);
    lua_settable(pLua_, -3);
}

void state::set_field_double(const std::string& sName, double fValue)
{
    lua_pushstring(pLua_, sName.c_str());
    lua_pushnumber(pLua_, fValue);
    lua_settable(pLua_, -3);
}

void state::set_field_string(const std::string& sName, const std::string& sValue)
{
    lua_pushstring(pLua_, sName.c_str());
    lua_pushstring(pLua_, sValue.c_str());
    lua_settable(pLua_, -3);
}

void state::set_field_bool(const std::string& sName, bool bValue)
{
    lua_pushstring(pLua_, sName.c_str());
    lua_pushboolean(pLua_, bValue);
    lua_settable(pLua_, -3);
}

void state::set_field_int(int iID, int iValue)
{
    lua_pushnumber(pLua_, iID);
    lua_pushnumber(pLua_, iValue);
    lua_settable(pLua_, -3);
}

void state::set_field_double(int iID, double fValue)
{
    lua_pushnumber(pLua_, iID);
    lua_pushnumber(pLua_, fValue);
    lua_settable(pLua_, -3);
}

void state::set_field_string(int iID, const std::string& sValue)
{
    lua_pushnumber(pLua_, iID);
    lua_pushstring(pLua_, sValue.c_str());
    lua_settable(pLua_, -3);
}

void state::set_field_bool(int iID, bool bValue)
{
    lua_pushnumber(pLua_, iID);
    lua_pushboolean(pLua_, bValue);
    lua_settable(pLua_, -3);
}

void state::set_top(uint uiSize)
{
    lua_settop(pLua_, uiSize);
}
}
