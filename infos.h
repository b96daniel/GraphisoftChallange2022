#ifndef INFOS_H
#define INFOS_H

class Infos {
    Infos() {}

public:
    int id{ -1 };
    int radius{ -1 };
    int gold{ -1 };
    int tick{ -1 };

    static Infos& get_instance()
    {
        static Infos instance;
        return instance;
    }

    Infos(Infos const&) = delete;
    void operator=(Infos const&) = delete;
};

#endif