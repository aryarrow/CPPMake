#pragma  once
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <utility>

//TODO:create a .at() and operator[] for the object and list
//this is because lockfile heavily relies on it



class variable {
public:
	//i love unique_ptr
	//but it has to go sadly
	//TODO:Transform this from std::unique_ptr<variable> to std::shared_ptr<variable>
	using list = std::vector<std::unique_ptr<variable>>;
	using object = std::unordered_map<std::string, std::unique_ptr<variable>>;

	using value_type = std::variant<
	int,
	bool,
	std::string,
	list,
	object
	>;

	value_type value;	
	
	std::unique_ptr<variable>& get(const std::string& tofind){
		if (!std::holds_alternative<object>(value)) {
			throw std::logic_error("Attempted to access a non object object");
		}
		
		auto& obj=std::get<object>(value);
		auto it=obj.find(tofind);

		if (it==obj.end()) {
			throw std::logic_error("Attempted to access an element that was not found");
		}
		return it->second;
	}

	template<class T> T& as(){
		if (!std::holds_alternative<T>(value)){
			throw std::runtime_error("Attempted to reach a different type than actually existing");
		}
		T& result=std::get<T>(value);
		return result;
	}

	std::string get_variable_type() const {
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
	variable(value_type construct_value):value(std::move(construct_value)){}
};
