#pragma once

#include <cstdint>
#include <memory>

class Win;
class Render;

class GameEngine
{
private:
    std::unique_ptr<Win> win_;
    std::unique_ptr<Render> render_;

    bool animating_{ false };

    GameEngine();
    GameEngine(GameEngine&) = delete;
    GameEngine(const GameEngine&&) = delete;
public:
    static GameEngine* inst();

    virtual bool init(uint32_t, uint32_t);
    virtual void run();
    virtual void destroy();

    void set_animating(bool);

    const Win& win() const;
    const Render& render() const;
};