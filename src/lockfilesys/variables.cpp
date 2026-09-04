#pragma  once
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <utility>
#include "variables.h"

std::shared_ptr<variable> variable::operator[](const std::string& index){
	return this->get(index);
}

std::shared_ptr<variable> variable::get(const std::string& index){
	if (!std::holds_alternative<object>(value)) {
		throw std::logic_error("Attempted to access a non object object");
	}
		
	auto& obj=std::get<object>(value);
	auto it=obj.find(index);

	if (it==obj.end()) {
		throw std::logic_error("Attempted to access an element that was not found");
	}
	return it->second;
}

template<class T> T& variable::as(){
	if (!std::holds_alternative<T>(value)){
		throw std::runtime_error("Attempted to reach a different type than actually existing");
	}
	
	T& result=std::get<T>(value);
	return result;
}

std::string variable::get_variable_type() const {
	if (std::holds_alternative<int>(value)) {
		return "int"; 
	}
	if (std::holds_alternative<bool>(value)) {
		return "bool";
	}
	if (std::holds_alternative<std::string>(value)) {
		return "string";
	}
	if (std::holds_alternative<list>(value)) {
		return "list";
	}
	if (std::holds_alternative<object>(value)) {
		return "object";
	}
	return "unknown";
}
