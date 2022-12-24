/****************************************************************************************************************************
  StringArray.h - Dead simple Ethernet AsyncWebServer.

  For W5500 LwIP Ethernet in ESP32 (ESP32 + W5500)

  AsyncWebServer_ESP32_W5500 is a library for the LwIP Ethernet W5500 in ESP32 to run AsyncWebServer

  Based on and modified from ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
  Built by Khoi Hoang https://github.com/khoih-prog/AsyncWebServer_ESP32_W5500
  Licensed under GPLv3 license

  Original author: Hristo Gochkov

  Copyright (c) 2016 Hristo Gochkov. All rights reserved.

  This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with this library;
  if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Version: 1.6.4

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.6.2   K Hoang      30/11/2022 Initial porting for ENC28J60 + ESP32. Sync with AsyncWebServer_WT32_ETH01 v1.6.2
  1.6.3   K Hoang      05/12/2022 Add Async_WebSocketsServer, MQTT examples
  1.6.4   K Hoang      23/12/2022 Remove unused variable to avoid compiler warning and error
 *****************************************************************************************************************************/

#ifndef STRINGARRAY_H_
#define STRINGARRAY_H_

#include "stddef.h"
#include "WString.h"

/////////////////////////////////////////////////

template <typename T>
class LinkedListNode
{
    T _value;

  public:
    LinkedListNode<T>* next;
    LinkedListNode(const T val): _value(val), next(nullptr) {}
    ~LinkedListNode() {}

    /////////////////////////////////////////////////

    inline const T& value() const
    {
      return _value;
    };

    /////////////////////////////////////////////////

    inline T& value()
    {
      return _value;
    }
};

/////////////////////////////////////////////////
/////////////////////////////////////////////////

template <typename T, template<typename> class Item = LinkedListNode>
class LinkedList
{
  public:
    typedef Item<T> ItemType;
    typedef std::function<void(const T&)> OnRemove;
    typedef std::function<bool(const T&)> Predicate;

  private:
    ItemType* _root;
    OnRemove _onRemove;

    class Iterator
    {
        ItemType* _node;

      public:
        Iterator(ItemType* current = nullptr) : _node(current) {}
        Iterator(const Iterator& i) : _node(i._node) {}

        /////////////////////////////////////////////////

        inline Iterator& operator ++()
        {
          _node = _node->next;
          return *this;
        }

        /////////////////////////////////////////////////

        inline bool operator != (const Iterator& i) const
        {
          return _node != i._node;
        }

        /////////////////////////////////////////////////

        inline const T& operator * () const
        {
          return _node->value();
        }

        /////////////////////////////////////////////////

        inline const T* operator -> () const
        {
          return &_node->value();
        }
    };

  public:
    typedef const Iterator ConstIterator;

    /////////////////////////////////////////////////

    inline ConstIterator begin() const
    {
      return ConstIterator(_root);
    }

    /////////////////////////////////////////////////

    inline ConstIterator end() const
    {
      return ConstIterator(nullptr);
    }

    /////////////////////////////////////////////////

    LinkedList(OnRemove onRemove) : _root(nullptr), _onRemove(onRemove) {}
    ~LinkedList() {}

    /////////////////////////////////////////////////

    void add(const T& t)
    {
      auto it = new ItemType(t);

      if (!_root)
      {
        _root = it;
      }
      else
      {
        auto i = _root;

        while (i->next)
          i = i->next;

        i->next = it;
      }
    }

    /////////////////////////////////////////////////

    inline T& front() const
    {
      return _root->value();
    }

    /////////////////////////////////////////////////

    inline bool isEmpty() const
    {
      return _root == nullptr;
    }

    /////////////////////////////////////////////////

    size_t length() const
    {
      size_t i = 0;
      auto it = _root;

      while (it)
      {
        i++;
        it = it->next;
      }

      return i;
    }

    /////////////////////////////////////////////////

    size_t count_if(Predicate predicate) const
    {
      size_t i = 0;
      auto it = _root;

      while (it)
      {
        if (!predicate)
        {
          i++;
        }
        else if (predicate(it->value()))
        {
          i++;
        }

        it = it->next;
      }

      return i;
    }

    /////////////////////////////////////////////////

    const T* nth(size_t N) const
    {
      size_t i = 0;
      auto it = _root;

      while (it)
      {
        if (i++ == N)
          return &(it->value());

        it = it->next;
      }

      return nullptr;
    }

    /////////////////////////////////////////////////

    bool remove(const T& t)
    {
      auto it = _root;
      auto pit = _root;

      while (it)
      {
        if (it->value() == t)
        {
          if (it == _root)
          {
            _root = _root->next;
          }
          else
          {
            pit->next = it->next;
          }

          if (_onRemove)
          {
            _onRemove(it->value());
          }

          delete it;
          return true;
        }

        pit = it;
        it = it->next;
      }

      return false;
    }

    /////////////////////////////////////////////////

    bool remove_first(Predicate predicate)
    {
      auto it = _root;
      auto pit = _root;

      while (it)
      {
        if (predicate(it->value()))
        {
          if (it == _root)
          {
            _root = _root->next;
          }
          else
          {
            pit->next = it->next;
          }

          if (_onRemove)
          {
            _onRemove(it->value());
          }

          delete it;
          return true;
        }

        pit = it;
        it = it->next;
      }

      return false;
    }

    /////////////////////////////////////////////////

    void free()
    {
      while (_root != nullptr)
      {
        auto it = _root;
        _root = _root->next;

        if (_onRemove)
        {
          _onRemove(it->value());
        }

        delete it;
      }

      _root = nullptr;
    }
};

/////////////////////////////////////////////////
/////////////////////////////////////////////////

class StringArray : public LinkedList<String>
{
  public:

    StringArray() : LinkedList(nullptr) {}

    /////////////////////////////////////////////////

    bool containsIgnoreCase(const String& str)
    {
      for (const auto& s : *this)
      {
        if (str.equalsIgnoreCase(s))
        {
          return true;
        }
      }

      return false;
    }
};

#endif /* STRINGARRAY_H_ */
