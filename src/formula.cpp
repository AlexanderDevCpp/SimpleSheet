#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression)
            :ast_(ParseFormulaAST(expression))
        {
        }

        Value Evaluate(const SheetInterface& sheet) const override {
            try {
                return ast_.Execute(sheet);
            }
            catch (const std::exception& ex) {
                return FormulaError(FormulaError::Category::Value);
            }
        }

        std::string GetExpression() const override {
            std::stringstream ss;
            ast_.PrintFormula(ss);
            return ss.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            std::set<Position> positions;

            for (auto& pos : ast_.GetCells()) {
                positions.insert(pos);
            }

            return { positions.begin(), positions.end()};
        }

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (const std::exception& ex) {
        throw FormulaException("some error");
    }

}