#ifndef ERROR_HPP
#define ERROR_HPP

#include <memory>
#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>

namespace cb
{
    namespace error
    {
        class error
        {
            struct error_data
            {
                error_data(const int code,const std::string &info):
                code_(code),
                info_(info)
                {
                }
                error_data::error_data(const int code,const std::string &info,const error &child):
                code_(code),
                info_(info),
                child_(child ? std::make_shared<const error>(child) : nullptr)
                {
                }
                const int code_;
                const std::string info_;
                const std::shared_ptr<const error> child_;
            };

            std::shared_ptr<error_data> data;
          public:
            error()
            {
            }
            error(const error &err):
            data(err.data)
            {
            }
            error(const error &&err):
            data(std::move(err.data))
            {
            }
            error(const int code,const std::string &info):
            data(std::make_shared<error_data>(code,info))
            {
            }
            error(const int code,const std::string &info,const error &child):
            data(std::make_shared<error_data>(code,info,child))
            {
            }

            error& operator=(const error &err)
            {
                data = err.data;
                return *this;
            }
            error& operator=(const error &&err)
            {
                data = std::move(err.data);
                return *this;
            }

            operator bool() const
            {
                return data;
            }
        
            int code() const
            {
                return data ? data->code_ : 0;
            }
            std::string info() const
            {
                return data ? data->info_ : "There isn't error.";
            }
            std::shared_ptr<const error> child()
            {
                return data ? data->child_ : std::shared_ptr<error>();
            }

            class iterator: public std::iterator<std::input_iterator_tag,error>
            {
                std::shared_ptr<const error> err;

                explicit iterator(const std::shared_ptr<const error> &err):
                err(err)
                {
                }
              public:
                iterator()
                {
                }
                iterator(const iterator &it):
                err(it.err)
                {
                }
                iterator(const iterator &&it):
                err(std::move(it.err))
                {
                }
                
                iterator& operator=(const iterator &it)
                {
                    err = it.err;
                    return *this;
                }
                iterator& operator=(const iterator &&it)
                {
                    err = std::move(it.err);
                    return *this;
                }

                bool exists()
                {
                    return err;
                }
                
                iterator& operator++()
                {
                    err = err->data->child_;
                    return *this; 
                }
                iterator operator++(int)
                {
                    iterator it = *this;
                    err = err->data->child_;
                    return it;
                }
                bool operator!=(const iterator &it) const
                {
                    return err != it.err;
                }
                bool operator==(const iterator &it) const
                {
                    return err == it.err;
                }
                const error& operator*() const
                {
                    return *err;
                }

                std::shared_ptr<const error> operator->() const
                {
                    return err;
                }

                friend class error;
            };

            iterator begin() const
            {
                return iterator(std::make_shared<const error>(*this));
            }
            iterator end() const
            {
                return iterator();
            }
        };

        static void print_errors(const error &err)
        {
            if(!err)
            {
                return;
            }
            int i = 0;
            std::for_each(err.begin(),
                          err.end(),
                          [&](const error &er)
                          {
                              if(!er)
                              {
                                  return;
                              }
                              std::cout.width(i+7);
                              std::cout << "code: '" << er.code() << "'" << std::endl;
                              std::cout.width(i+7);
                              std::cout << "info: \"" << er.info() << "\"" << std::endl;
                              i+=2;
                          });
        }
    };
};

#endif // ERROR_HPP