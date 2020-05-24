//
// Created by andy on 20-5-23.
//

#ifndef WEBSERVER_NONCOPYABLE_H
#define WEBSERVER_NONCOPYABLE_H


namespace reactor {

    class noncopyable {
    public:
        noncopyable(const noncopyable &) = delete;

        void operator=(const noncopyable &) = delete;

    protected:
        noncopyable() = default;

        ~noncopyable() = default;
    };

}  // namespace reator



#endif //WEBSERVER_NONCOPYABLE_H
