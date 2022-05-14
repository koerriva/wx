#pragma once

#include <sol/sol.hpp>
#include <functional>

namespace wx
{
    class ScriptEngine{
        public:
            static void Init(){
                _instance = new ScriptEngine();
                _instance->init();
            }

            template<class R, class... ARGS>
            static std::function<R(ARGS...)> Run(std::string code, const char* fn){
                return _instance->run(code,fn);
            }
            static void Cleanup(){
                delete _instance;
            }
        private:
            static ScriptEngine* _instance;
            std::unordered_map<std::string,sol::load_result> load_scripts;

            sol::state lua;

            ScriptEngine();
            ~ScriptEngine();

            void init();

            auto run(std::string code, const char* function){
                auto result = lua.safe_script(code,sol::script_throw_on_error);
                if (!result.valid()) {
                    sol::error err = result;
                    std::cerr << "The code has failed to run!\n"
                            << err.what() << "\nPanicking and exiting..."
                            << std::endl;
                    throw std::runtime_error(err.what());
                }
                return lua[function];
            }

            void cleanup();
    };
} // namespace wx
