#pragma once

#include "cell.h"
#include "common.h"

#include <vector>
#include <functional>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void UpdateSize();

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;

    void PrintTexts(std::ostream& output) const override;

private:

    std::unordered_map<Position, std::set<Position>, PositionHasher> position_to_dependent_;

    std::unordered_map<Position, Cell, PositionHasher> cells_;

    Size size_;
};