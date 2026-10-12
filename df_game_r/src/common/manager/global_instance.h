#ifndef OLDSRV_GLOBAL_GLOBAL_INSTANCE_H_
#define OLDSRV_GLOBAL_GLOBAL_INSTANCE_H_


template <typename T>
class GlobalInstance {
public:
    static T* inst_ptr();
    static T* create();
    static T* inst();
    static void destroy();

private:
    static T* m_p;
};


#endif  // OLDSRV_GLOBAL_GLOBAL_INSTANCE_H_
