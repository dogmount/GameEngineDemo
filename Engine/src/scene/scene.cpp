#include "scene/scene.h"
#include "scene/components/animationComponent.h"
#include "scene/components/cameraComponent.h"
#include "scene/components/lightComponent.h"
#include "scene/components/meshComponent.h"
#include "scene/components/physicsComponent.h"
#include "scene/components/playerControlComponent.h"
#include "scene/components/audioComponent.h"
#include "scene/components/audioListenerComponent.h"
#include "scene/components/spriteComponent.h"
#include "scene/components/ui/UIElementComponent.h"
#include "scene/components/ui/canvasComponent.h"
#include "scene/components/ui/textComponent.h"
#include "scene/components/ui/buttonComponent.h"
#include "engine/Engine.h"

namespace GL_engine
{
    //  静态注册
    void Scene::RegisterTypes()
    {
        AnimationComponent::Register();
        CameraComponent::Register();
        LightComponent::Register();
        MeshComponent::Register();
        PhysicsComponent::Register();
        PlayerControlComponent::Register();
        AudioComponent::Register();
        AudioListenerComponent::Register();
        SpriteComponent::Register();
        UIElementComponent::Register();
        CanvasComponent::Register();
        TextComponent::Register();
        ButtonComponent::Register();
    }

    //  更新
    void Scene::Update(float deltaTime)
    {
        m_objects.erase(
            std::remove_if(m_objects.begin(), m_objects.end(),
                           [](const std::unique_ptr<GameObject> &obj)
                           { return !obj->IsAlive(); }),
            m_objects.end());
        /*
            remove_if将所有满足条件的元素移动到容器的末尾，并返回一个指向第一个被移动元素的迭代器
        */

        for (auto &obj : m_objectsToAdd)
        {
            SetParent(obj.first, obj.second);
        }
        m_objectsToAdd.clear();

        //  设置更新进行状态
        m_isUpdating = true;

        for (auto it = m_objects.begin(); it != m_objects.end();)
        {
            if ((*it)->IsAlive())
            {
                (*it)->Update(deltaTime);
                ++it;
            }
            else
            {
                it = m_objects.erase(it);
            }
        }

        //  更新结束后恢复默认更新进行状态
        m_isUpdating = false;
    }

    //  清除
    void Scene::Clear()
    {
        m_objects.clear();
    }

    //  游戏对象创建方法
    GameObject *Scene::CreateObject(const ::std::string &name, GameObject *parent)
    {
        auto obj = new GameObject();
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

    //  按类型创建
    GameObject *Scene::CreateObject(const ::std::string &type, const ::std::string &name, GameObject *parent)
    {
        auto obj = GameObjectFactory::GetInstance().GameObjectCreate(type);
        if (obj)
        {
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
        }

        return obj;
    }

    //  设置父对象
    bool Scene::SetParent(GameObject *obj, GameObject *parent)
    {
        bool result = false;
        auto currentParent = obj->GetParent();

        //  parent为空
        if (parent == nullptr)
        {
            //  当前obj有父对象
            if (currentParent != nullptr)
            {
                auto it = std::find_if(
                    currentParent->m_children.begin(),
                    currentParent->m_children.end(),
                    [obj](const std::unique_ptr<GameObject> &el)
                    {
                        return el.get() == obj;
                    });

                if (it != currentParent->m_children.end())
                {
                    m_objects.push_back(std::move(*it));
                    obj->m_parent = nullptr;
                    currentParent->m_children.erase(it);
                    result = true;
                }
            }
            //  当前obj无父对象---1、对象在场景根目录    2、对象刚刚被创建
            else
            {
                //  在根目录中搜索
                auto it = std::find_if(
                    m_objects.begin(),
                    m_objects.end(),
                    [obj](const std::unique_ptr<GameObject> &el)
                    {
                        return el.get() == obj;
                    });

                //  对象刚创建
                if (it == m_objects.end())
                {
                    std::unique_ptr<GameObject> objPtr(obj);
                    m_objects.push_back(std::move(objPtr));
                    result = true;
                }
            }
        }
        //  parent非空
        else
        {
            //  obj有父对象
            if (currentParent != nullptr)
            {
                auto it = std::find_if(
                    currentParent->m_children.begin(),
                    currentParent->m_children.end(),
                    [obj](const std::unique_ptr<GameObject> &el)
                    {
                        return el.get() == obj;
                    });

                if (it != currentParent->m_children.end())
                {
                    //  确保当前对象父子关系不产生循环
                    bool found = false;
                    auto currentElement = parent;
                    while (currentElement)
                    {
                        if (currentElement == obj)
                        {
                            found = true;
                            break;
                        }
                        currentElement = currentElement->GetParent();
                    }

                    //  如果obj未发生循环父子关系
                    if (!found)
                    {
                        parent->m_children.push_back(std::move(*it));
                        obj->m_parent = parent;
                        currentParent->m_children.erase(it);
                        result = true;
                    }
                }
            }
            //  当前obj无父对象---1、对象在场景根目录    2、对象刚刚被创建
            else
            {
                //  在根目录中搜索
                auto it = std::find_if(
                    m_objects.begin(),
                    m_objects.end(),
                    [obj](const std::unique_ptr<GameObject> &el)
                    {
                        return el.get() == obj;
                    });

                //  对象刚创建
                if (it == m_objects.end())
                {
                    std::unique_ptr<GameObject> objPtr(obj);
                    parent->m_children.push_back(std::move(objPtr));
                    obj->m_parent = parent;
                    result = true;
                }
                //  对象在根目录
                else
                {
                    //  确保当前对象父子关系不产生循环
                    bool found = false;
                    auto currentElement = parent;
                    while (currentElement)
                    {
                        if (currentElement == obj)
                        {
                            found = true;
                            break;
                        }
                        currentElement = currentElement->GetParent();
                    }

                    if (!found)
                    {
                        parent->m_children.push_back(std::move(*it));
                        obj->m_parent = parent;
                        m_objects.erase(it);
                        result = true;
                    }
                }
            }
        }
        return result;
    }

    //  按名称查找对象
    GameObject *Scene::FindObjectByName(const std::string &name)
    {
        for (auto &obj : m_objects)
        {
            if (auto child = obj->FindChildByName(name))
            {
                return child;
            }
        }
        return nullptr;
    }

    //  设置/获取主相机
    void Scene::SetMainCamera(GameObject *camera)
    {
        m_mainCamera = camera;
    }
    GameObject *Scene::GetMainCamera()
    {
        return m_mainCamera;
    }

    //  管理光照信息
    std::vector<LightData> Scene::CollectLights()
    {
        std::vector<LightData> lights;
        for (auto &obj : m_objects)
        {
            CollectLightsRecursive(obj.get(), lights);
        }
        return lights;
    }
    //  辅助遍历游戏对象获取光照数组
    void Scene::CollectLightsRecursive(GameObject *obj, std::vector<LightData> &out)
    {
        if (auto light = obj->GetComponent<LightComponent>())
        {
            LightData data;
            data.color = light->GetColor();
            data.position = obj->GetWorldPosition();

            //  ---- 阴影: 灯声明意图, 这里推导成帧数据 ----
            data.castsShadow = light->GetCastsShadow();
            if (data.castsShadow)
            {
                data.shadow = ShadowCamera::DirectionalLight(
                    data.position,
                    light->GetShadowCenter(),
                    light->GetShadowRadius());
            }

            out.push_back(data);
        }

        for (auto &child : obj->m_children)
        {
            CollectLightsRecursive(child.get(), out);
        }
    }

    //  场景加载
    std::shared_ptr<Scene> Scene::Load(const std::string &path)
    {
        const std::string contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);
        if (contents.empty())
        {
            return nullptr;
        }

        auto json = nlohmann::json::parse(contents);
        if (json.empty())
        {
            return nullptr;
        }

        auto result = std::make_shared<Scene>();

        const std::string sceneName = json.value("name", "noname");

        if (json.contains("objects") && json["objects"].is_array())
        {
            const auto &objects = json["objects"];

            for (const auto &obj : objects)
            {
                result->LoadObject(obj, nullptr);
            }
        }

        if (json.contains("camera"))
        {
            std::string cameraObjName = json.value("camera", "");
            for (const auto &child : result->m_objects)
            {
                if (auto object = child->FindChildByName(cameraObjName))
                {
                    result->SetMainCamera(object);
                    break;
                }
            }
        }

        std::string activeCanvasName = json.value("activeCanvas", "");
        for (auto &child : result->m_objects)
        {
            if (auto canvasObject = child->FindChildByName(activeCanvasName))
            {
                if (auto component = canvasObject->GetComponent<CanvasComponent>())
                {
                    Engine::GetInstance().GetUIInputSystem().SetCanvas(component);
                    break;
                }
            }
        }

        return result;
    }
    //  辅助场景加载--场景对象加载
    void Scene::LoadObject(const nlohmann::json &jsonObject, GameObject *parent)
    {
        const std::string name = jsonObject.value("name", "Object");

        GameObject *gameObject = nullptr;

        if (jsonObject.contains("type"))
        {
            const std::string type = jsonObject.value("type", "");
            if (type == "gltf")
            {
                // gltf加载
                std::string path = jsonObject.value("path", "");
                gameObject = GameObject::LoadGLTF(path, this);
                if (gameObject)
                {
                    gameObject->SetParent(parent);
                    gameObject->SetName(name);
                }
            }
            else
            {
                gameObject = CreateObject(type, name, parent);
            }
        }
        else
        {
            gameObject = CreateObject(name, parent);
        }

        if (!gameObject)
        {
            return;
        }

        //  读取变换
        if (jsonObject.contains("position"))
        {
            auto posObj = jsonObject["position"];
            glm::vec3 pos;
            pos.x = posObj.value("x", 0.0f);
            pos.y = posObj.value("y", 0.0f);
            pos.z = posObj.value("z", 0.0f);
            gameObject->SetPosition(pos);
        }

        if (jsonObject.contains("rotation"))
        {
            auto rotObj = jsonObject["rotation"];
            // glm::vec3 rotVec3;
            // glm::quat rot;
            // rotVec3.x = rotObj.value("x", 0.0f);
            // rotVec3.y = rotObj.value("y", 0.0f);
            // rotVec3.z = rotObj.value("z", 0.0f);
            // rot.w = rotObj.value("w", 1.0f);
            // gameObject->SetRotation(glm::quat(glm::radians(glm::vec3(rotVec3))));

            glm::quat rot;
            rot.x = rotObj.value("x", 0.0f);
            rot.y = rotObj.value("y", 0.0f);
            rot.z = rotObj.value("z", 0.0f);
            rot.w = rotObj.value("w", 1.0f);
            gameObject->SetRotation(rot);
        }

        //  新增: 角度制欧拉(YXZ), 便于手写场景
        if (jsonObject.contains("rotationEuler"))
        {
            auto euler = jsonObject["rotationEuler"];
            gameObject->SetRotationEulerYXZ(glm::vec3(euler.value("x", 0.0f), euler.value("y", 0.0f), euler.value("z", 0.0f)));
        }

        if (jsonObject.contains("scale"))
        {
            auto scaleObj = jsonObject["scale"];
            glm::vec3 scale;
            scale.x = scaleObj.value("x", 1.0f);
            scale.y = scaleObj.value("y", 1.0f);
            scale.z = scaleObj.value("z", 1.0f);
            gameObject->SetScale(scale);
        }

        //  加载属性
        gameObject->LoadProperties(jsonObject);

        if (jsonObject.contains("components") && jsonObject["components"].is_array())
        {
            const auto &components = jsonObject["components"];
            for (const auto &comp : components)
            {
                const std::string type = comp.value("type", "");
                Component *component = ComponentFactory::GetInstance().ComponentCreate(type);
                if (component)
                {
                    component->LoadProperties(comp);
                    gameObject->AddComponent(component);
                }
            }
        }

        if (jsonObject.contains("children") && jsonObject["children"].is_array())
        {
            const auto &children = jsonObject["children"];
            for (const auto &child : children)
            {
                LoadObject(child, gameObject);
            }
        }

        gameObject->Init();
    }
}