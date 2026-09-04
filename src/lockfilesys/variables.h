#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <variant>
#include <vector>

class variable{
public:
	using list=std::vector<std::shared_ptr<variable>>;
	using object=std::unordered_map<std::string, std::shared_ptr<variable>>;

	using value_type=std::variant<
	int,
	bool,
	std::string,
	list,
	object
	>;

	value_type value;

	
	std::shared_ptr<variable> operator[](const std::string& index);
	std::shared_ptr<variable> get(const std::string& index);
	template<class T> T& as();
	std::string get_variable_type() const;
	variable(value_type construct_value):value(std::move(construct_value)){}
};
