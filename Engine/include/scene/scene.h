#ifndef SCENE_H
#define SCENE_H
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "scene/gameObject.h"
#include "engine/common.h"

namespace GL_engine
{
    class GameObject;

    class Scene
    {
    public:
        //  静态注册
        static void RegisterTypes();

        //  更新
        void Update(float deltaTime);

        //  清除
        void Clear();

        //  游戏对象创建方法
        GameObject *CreateObject(const std::string &name, GameObject *parent = nullptr);
        GameObject *CreateObject(const std::string &type, const std::string &name, GameObject *parent = nullptr); //  按类型创建

        //  模板化方法创建游戏对象
        template <typename T, typename = typename std::enable_if_t<std::is_base_of_v<GameObject, T>>>
        T *CreateObject(const std::string &name, GameObject *parent = nullptr)
        {
            auto obj = new T();
            obj->SetName(name);
            obj->m_scene = this;

            //  避免在更新进行时添加对象
            if (m_isUpdating)
            {
                m_objectsToAdd.push_back({obj, parent});
            }
            else
            {
                SetParent(obj, parent);
            }

            return obj;
        }
        /*
            typename = typename std::enable_if_t<std::is_base_of_v<GameObject, T>>
            即：确保模板T是继承自GameObject类时，才允许调用该构建函数

            std::enable_if_t<condition>（C++14）：
            如果 condition 为 true，则 enable_if_t 被定义为类型 void；如果为 false，则模板替换失败

            std::is_base_of_v<GameObject, T>（C++17）：
            是一个编译期常量（true 或 false），检查 GameObject 是不是 T 的基类
        */

        //  设置父对象
        bool SetParent(GameObject *obj, GameObject *parent);

        //  按名称查找对象
        GameObject *FindObjectByName(const std::string &name);

        //  设置/获取主相机
        void SetMainCamera(GameObject *camera);
        GameObject *GetMainCamera();

        //  管理光照信息
        std::vector<LightData> CollectLights();

        //  场景加载
        static std::shared_ptr<Scene> Load(const std::string &path);

    private:
        void CollectLightsRecursive(GameObject *obj, std::vector<LightData> &out); //  辅助遍历游戏对象获取光照数组

        //  辅助场景加载--场景对象加载
        void LoadObject(const nlohmann::json &jsonObject, GameObject *parent);

    private:
        std::vector<std::unique_ptr<GameObject>> m_objects;                //  游戏对象数组
        std::vector<std::pair<GameObject *, GameObject *>> m_objectsToAdd; //  待添加对象数组
        GameObject *m_mainCamera = nullptr;                                //  相机对象

        bool m_isUpdating = false; //  场景更新进行状态
    };
}

#endif