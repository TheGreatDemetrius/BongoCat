#pragma once
#include <utility>

// Base RAII wrapper
template<typename T, typename Deleter>
class BaseRAIIWrapper {
protected:
	T resource_;
	bool isOwned_;

public:
	BaseRAIIWrapper() : resource_(nullptr), isOwned_(false) {}
	BaseRAIIWrapper(T resource, bool owned = false)
		: resource_(resource), isOwned_(owned) {
	}

	virtual ~BaseRAIIWrapper() {
		if (isOwned_ && resource_) {
			Deleter()(resource_);
		}
	}

	// Non-copyable
	BaseRAIIWrapper(const BaseRAIIWrapper&) = delete;
	BaseRAIIWrapper& operator=(const BaseRAIIWrapper&) = delete;

	// Movable
	BaseRAIIWrapper(BaseRAIIWrapper&& other) noexcept
		: resource_(std::exchange(other.resource_, nullptr))
		, isOwned_(std::exchange(other.isOwned_, false)) {
	}

	BaseRAIIWrapper& operator=(BaseRAIIWrapper&& other) noexcept {
		if (this != &other) {
			if (isOwned_ && resource_) {
				Deleter()(resource_);
			}
			resource_ = std::exchange(other.resource_, nullptr);
			isOwned_ = std::exchange(other.isOwned_, false);
		}
		return *this;
	}

	// Accessors
	T get() const { return resource_; }
	bool isValid() const { return resource_ != nullptr; }
	explicit operator bool() const { return isValid(); }
};
