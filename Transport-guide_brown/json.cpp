#include "json.h"

using namespace std;

namespace Json {

    Document::Document(Node root) : root(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root;
    }

    Node LoadNode(istream& input);

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
    // добавить bool "is_roundtrip": true

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
        } else {
            input.putback(c);
            return LoadNumber(input);
        }
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

}
