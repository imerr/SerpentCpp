# SerpentCpp
C++11 Serpent Loader

Loads basic serpent strings

### Limitations:
* Numbers are stored as double - so a loss of precision may happen
* Loads tables in a recursive manner - so it might cause a stack overflow if it's nested too deeply
* This was written to load fairly basic tables - so more specific lua things might break. Do tell if they do and I'll look at fixing things

### Usage
```c++
std::string serpentString = "do local _={keyHere=5,{wowNested={moreNesting=\"string\",false=true}}}; return _; end";
try {
	Serpent serpent = Serpent::Load(serpentString);
	Serpent& someChild = serpent["keyHere"];
	if (!someChild.IsNull()) {
		if (someChild.IsNumber()) {
			std::cout << "'keyHere' is " << someChild.As<double>() << std::endl;
		} else {
			std::cerr << "'keyHere' is not a number :( "<< someChild.Type()  << std::endl;
		}
	} else {
		std::cerr << "'keyHere' does not exist" << std::endl;
	}
	if (serpent[1]["wowNested"][false].IsBool() && serpent[1]["wowNested"][false].As<bool>()) {
		std::cout << "Nesting and indexing by bool seems to work fine too" << std::endl;
	} else {
		std::cerr << "Nesting and indexing by bool is broken :(" << std::endl;
	}
} catch (Serpent::Exception e) {
	// Uh Oh - something went wrong
	std::cerr << "Something went: " << std::endl
			  << e.what() << std::endl;
}
```