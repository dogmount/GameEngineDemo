#include "game.h"
#include "engine/eng.h"

int main()
{
    //  创建游戏实例
    Game *game = new Game();

    //  单例模式创建引擎实例
    GL_engine::Engine &engine = GL_engine::Engine::GetInstance();

    //  设置应用程序并传递游戏
    engine.SetApp(game);

    //  初始化成功则进入主循环
    if (engine.EngInit(1920, 1080))
    {
        engine.EngRun();
    }

    //  回收资源
    engine.EngDestroy();
    return 0;
}