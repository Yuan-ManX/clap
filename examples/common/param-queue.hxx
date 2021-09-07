#pragma once

#include <cassert>

#include "param-queue.hh"

template<typename T>
ParamQueue<T>::ParamQueue() { reset(); }

template<typename T>
void ParamQueue<T>::reset() {
   for (auto &q : _queues)
      q.clear();

   _free = &_queues[0];
   _producer = &_queues[1];
   _consumer = nullptr;
}

template<typename T>
void ParamQueue<T>::setCapacity(size_t capacity) {
   for (auto &q : _queues)
      q.reserve(2 * capacity);
}

template<typename T>
void ParamQueue<T>::set(clap_id id, const value_type& value) {
   _producer.load()->emplace(id, value);
}

template<typename T>
void ParamQueue<T>::producerDone() {
   if (_consumer)
      return;

   _consumer.store(_producer.load());
   _producer.store(_free.load());
   _free.store(nullptr);

   assert(_producer);
}

template<typename T>
void ParamQueue<T>::consume(const std::function<void(clap_id, const value_type& value)> consumer) {
   assert(consumer);

   if (!_consumer)
      return;

   for (auto &x : *_consumer)
      consumer(x.first, x.second);

   _consumer.load()->clear();
   if (_free)
      return;

   _free = _consumer.load();
   _consumer = nullptr;
}
