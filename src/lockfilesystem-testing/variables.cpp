#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <utility>

class variable {
public:
	using list = std::vector<std::unique_ptr<variable>>;//i love unique_ptr
	using object = std::unordered_map<std::string, std::unique_ptr<variable>>;

	using value_type = std::variant<
	int,
	bool,
	std::string,
	list,
	object
	>;

	value_type value;	

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
