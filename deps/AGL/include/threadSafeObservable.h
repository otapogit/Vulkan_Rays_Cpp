#pragma once

#include <iostream>
#include <functional>
#include <list>
#include <mutex>

namespace autis {

	template <typename T>
	class ThreadSafeObservable {
	public:
		typedef typename std::list<std::function<void(T)>>::iterator SubscriptionId;
		/**
		Registra un oyente.
		\return Un identificador para poder cancelar la subscripción más adelante, usando removeListener
		*/
		SubscriptionId addListener(std::function<void(T)> listener) {
			std::lock_guard<std::mutex> lock(mutex);
			listeners.push_back(listener);
			return std::prev(listeners.end());
		}

		void removeListener(SubscriptionId idListener) {
			std::lock_guard<std::mutex> lock(mutex);
			if (!listeners.empty())
				listeners.erase(idListener);
		}

		void notify(T value) {
			std::lock_guard<std::mutex> lock(mutex);
			for (auto l : listeners) {
				l(value);
			}
		};
	private:
		std::list <std::function<void(T)>> listeners;
		std::mutex mutex;
	};

};
