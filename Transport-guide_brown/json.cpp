#include "json.h"

using namespace std;

namespace Json {

    Document::Document(Node root) : root(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root;
    }

    Node LoadNode(istream& input);

    //---------------- JSON input -------------------------------------------------------

    Node LoadArray(istream& input) {
        vector<Node> result;

        for (char c; input >> c && c != ']'; ) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadInt(istream& input) {
        int result = 0;

        while (isdigit(input.peek())) {
            result *= 10;
            result += input.get() - '0';
        }

        return Node(result);
    }

    Node LoadNumber(istream& input) {
        auto integer(LoadInt(input));

        if (input.peek() != '.') {  // число целое
            return Node(integer);
        }

        input.get();
        auto fractional(LoadInt(input));

        int counterDecemal = 1;
        for (int i(fractional.AsInt()); i != 0; i /= 10) { 
            counterDecemal *= 10;
        }
        
        return Node(double(integer.AsInt()) + (fractional.AsInt() / double(counterDecemal)));
    }
    
    Node LoadBool(istream& input) {
        string line;
        getline(input, line, 'e');

        //input.get(); //get 'e'

        //char c;
        //input >> c;

        if (line == "tru") {
            return Node(bool(true));
        } else { // (line == "fals")
            return Node(bool(false));
        }
        // не очень надежно, но пока так
    }

    Node LoadString(istream& input) {
        string line;
        getline(input, line, '"');
        return Node(move(line));
    }

    Node LoadDict(istream& input) {
        map<string, Node> result;

        for (char c; input >> c && c != '}'; ) {
            if (c == ',') {
                input >> c;
            }

            string key = LoadString(input).AsString();
            input >> c;
            result.emplace(move(key), LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadNode(istream& input) {
        char c;
        input >> c;

        if (c == '[') {
            return LoadArray(input);
        } else if (c == '{') {
            return LoadDict(input);
        } else if (c == '"') {
            return LoadString(input);
        } else if (c == 't' || c == 'f') {
            input.putback(c);
            return LoadBool(input);
        } else {
            input.putback(c);
            return LoadNumber(input);
        }
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    //---------------- JSON output ------------------------------------------------------

    ostream& UnloadNode(ostream& out, const Node& node, string tab = string(""));

    ostream& UnloadArray(ostream& out, const Node& node, string tab = string("")) {
        const vector<Node>& array_ = node.AsArray();

        size_t arraySize = array_.size();
        for (const auto& includeNode : array_) {
            out << tab;
            UnloadNode(out, includeNode, tab);
            out << (--arraySize ? ",\n" : "\n");
        }

        return out;
    }

    ostream& UnloadDict(ostream& out, const Node& node, string tab = string("")) {
        const map<std::string, Node>& dict = node.AsMap();

        size_t dictSize = dict.size();
        for (const auto& [name, includeNode] : dict) {
            out << tab << "\"" << name << "\": ";
            UnloadNode(out, includeNode, tab);
            out << (--dictSize ? ",\n" : "\n");
        }

        return out;
    }

    ostream& UnloadNode(ostream& out, const Node& node, string tab) {
        size_t index = node.GetVariantIndex();

        if (index == 0) {
            out << "[\n";
            UnloadArray(out, node, string(tab + "  "));
            return out << tab << "]";
        }
        else if (index == 1) {
            out << "{\n";
            UnloadDict(out, node, string(tab + "  "));
            return out << tab << "}";
        }
        else if (index == 2) {
            return out << node.AsInt();
        }
        else if (index == 3) {
            return out << node.AsDouble();
        }
        else if (index == 4) {
            return out << node.AsBool();
        }
        else {
            return out << "\"" << node.AsString() << "\"";
        }
        // тоже не очень надежная обработка...
    }

    ostream& UnloadDoc(ostream& out, const Document& doc) {
        //out.precision(6);
        UnloadNode(out, doc.GetRoot());
        return out;
    }
}
