#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, CellInterface::Value value) {
	if (value.index() == 0) {
		output << std::get<std::string>(value);
	}
	if (value.index() == 1) {
		output << std::get<double>(value);
	}
	if (value.index() == 2) {
		output << std::get<FormulaError>(value).ToString();
	}

	return output;
}

void Sheet::SetCell(Position pos, std::string text) {

	if (!pos.IsValid()) {
		throw InvalidPositionException("Position is not valid"s);
	}

	if (size_.rows < pos.row) {
		size_.rows = pos.row;
	}
	if (size_.cols < pos.col) {
		size_.cols = pos.col;
	}

	Cell new_cell(*this);

	new_cell.Set(text);

	if (cells_.count(pos) != 0) {
		auto back_cells = cells_[pos].GetBackCells();

		for (auto& cell_pos : back_cells) {
			if (position_to_dependent_.count(cell_pos)) {
				position_to_dependent_.at(cell_pos).erase(pos);
			}
		}
	}

	if (text[0] == FORMULA_SIGN && text.size() > 1) {
		auto formula = ParseFormula(text.substr(1));

		for (Position cell_pos : formula->GetReferencedCells()) {

			if (cell_pos == pos) {
				throw CircularDependencyException("ref to this");
			}
			else {
				new_cell.InsertNextCell(cell_pos);
			}

			if (cells_.count(cell_pos) == 0) {
				position_to_dependent_[cell_pos].insert(pos);
				Cell temp_cell(*this);
				temp_cell.InsertBackCell(pos);
				cells_[cell_pos] = std::move(temp_cell);
			}
			else {
				position_to_dependent_[cell_pos].insert(pos);
				cells_[cell_pos].InsertBackCell(pos);
			}

		}
	}

	new_cell.CheckForDependencies(pos);


	if (position_to_dependent_.count(pos)) {
		for (auto& cell_pos : position_to_dependent_.at(pos)) {
			cells_[cell_pos].InvalidateCache();
		}
	}

	cells_[pos] = std::move(new_cell);

}

const CellInterface* Sheet::GetCell(Position pos) const {
	if (!pos.IsValid()) {
		throw InvalidPositionException("Position is not valid"s);
	}

	auto temp_cell = cells_.find(pos);

	if (temp_cell != cells_.end()) {
		return &temp_cell->second;
	}

	return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {

 	if (!pos.IsValid()) {
		throw InvalidPositionException("Position is not valid"s);
	}

	auto temp_cell = cells_.find(pos);
	if (temp_cell != cells_.end()) {
		return &temp_cell->second;
	}

	return nullptr;
}

void Sheet::ClearCell(Position pos) {
	if (!pos.IsValid()) {
		throw InvalidPositionException("Position is not valid"s);
	}

	cells_.erase(pos);
	UpdateSize();
}

Size Sheet::GetPrintableSize() const {
	if (cells_.empty()) {
		return {0,0};
	}
	return { size_.rows + 1, size_.cols + 1 };
}

void Sheet::PrintValues(std::ostream& output) const {
	Position pos;

	if (!cells_.empty()) {

		Position pos;

		for (int row = 0; row <= size_.rows; row++) {
			pos.row = row;

			for (int col = 0; col <= size_.cols; col++) {
				pos.col = col;

				if (cells_.count(pos)) {
					output << cells_.at(pos).GetValue();
				}
				if (col != (size_.cols)) {
					output << '\t';
				}
			}
			output << '\n';
		}
	}
}


void Sheet::PrintTexts(std::ostream& output) const {
	if (!cells_.empty()) {

		Position pos;

		for (int row = 0; row <= size_.rows; row++) {
			pos.row = row;

			for (int col = 0; col <= size_.cols; col++) {
				pos.col= col;

				if (cells_.count(pos)) {
					output << cells_.at(pos).GetText();
				}
				if (col != (size_.cols)) {
					output << '\t';
				}
			}
			output << '\n';
		}
	}
}

void Sheet::UpdateSize() {
	int col = 0;
	int row = 0;

	for (auto& [pos,_] : cells_) {
		if (pos.col > col) {
			col = pos.col;
		}
		if (pos.row > row) {
			row = pos.row;
		}
	}

	size_.cols = col;
	size_.rows = row;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
