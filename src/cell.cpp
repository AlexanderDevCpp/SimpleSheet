#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


Cell::Cell(SheetInterface& sheet)
	:sheet_(&sheet), impl_(std::make_unique<EmptyImpl>())
{}

Cell::Cell()
	:impl_(std::make_unique<EmptyImpl>())
{}

void Cell::Set(std::string text) {
	if (text[0] == FORMULA_SIGN && text.size() > 1) {
		auto formula = ParseFormula(text.substr(1));

		impl_ = std::make_unique<FormulaImpl>(std::move(formula),sheet_);
	}
	else {
		impl_ = std::make_unique<TextImpl>(text);
	}


	is_valid_ = false;
}

void Cell::Clear() {
	impl_.reset();
}

Cell::Value Cell::GetValue() const {
	if (is_valid_) {
		return cache_;
	}

	const_cast<Cell::Value&>(cache_) = impl_->GetValue();
	const_cast<bool&>(is_valid_) = true;
	return cache_;
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const
{
	return { next_cells_.begin(),next_cells_.end() };
}

void Cell::InvalidateCache() {
	for (auto cell_pos : back_cells_) {
		Cell* cell = reinterpret_cast<Cell*>(sheet_->GetCell(cell_pos));
		if (cell != nullptr) {
			cell->InvalidateCache();
		}
	}

	is_valid_ = false;
}

void Cell::CheckForDependencies(Position pos) const {
	for (auto cell_pos : next_cells_) {
		if (cell_pos == pos) {
			throw CircularDependencyException("circular dependency");
		}

		Cell* cell = reinterpret_cast<Cell*>(sheet_->GetCell(cell_pos));
		cell->CheckForDependencies(pos);
	}

}

void Cell::InsertNextCell(Position pos) {
	next_cells_.insert(pos);
}

void Cell::InsertBackCell(Position pos) {
	back_cells_.insert(pos);
}

std::set<Position> Cell::GetBackCells() {
	return next_cells_;
}


CellInterface::Value Cell::EmptyImpl::GetValue() const {
	return 0.0;
}

std::string Cell::EmptyImpl::GetText() const {
	return {};
}

CellInterface::Value Cell::TextImpl::GetValue() const {
	if (text_[0] == ESCAPE_SIGN) {
		return text_.substr(1);
	}
	return text_;
}

std::string Cell::TextImpl::GetText() const {
	return text_;
}

CellInterface::Value Cell::FormulaImpl::GetValue() const {
	try {
		return std::get<double>(formula_->Evaluate(*sheet_));
	}
	catch (FormulaError ex) {
		return ex;
	}
}

std::string Cell::FormulaImpl::GetText() const {
	return FORMULA_SIGN + formula_->GetExpression();
}
