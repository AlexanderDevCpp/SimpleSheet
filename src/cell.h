#pragma once

#include "common.h"
#include "formula.h"

#include <set>

struct PositionHasher
{
    size_t operator() (const Position& pos) const {

        size_t col = int_hasher_(pos.col);
        size_t row = int_hasher_(pos.row);
        return col + row * 37;
    }
private:
    std::hash<int> int_hasher_;
};


class Cell : public CellInterface {
public:
    Cell(SheetInterface&);
    Cell();
    ~Cell() = default;

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    void InvalidateCache();
    void CheckForDependencies(Position) const;

    void InsertNextCell(Position);
    void InsertBackCell(Position);
    std::set<Position> GetBackCells();

    Cell& operator=(Cell&& other) noexcept {
        this->impl_ = std::move(other.impl_);
        this->cache_ = std::move(other.cache_);
        this->is_valid_ = std::move(other.is_valid_);
        this->back_cells_ = std::move(other.back_cells_);
        this->next_cells_ = std::move(other.next_cells_);
        this->sheet_ = std::move(other.sheet_);

        return *this;
    }

private:

    class Impl {
    public:
        virtual CellInterface::Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        CellInterface::Value GetValue() const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string text)
            :text_(text)
        {

        }
        CellInterface::Value GetValue() const override;
        std::string GetText() const override;

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::unique_ptr<FormulaInterface>&& formula, SheetInterface* sheet)
            : sheet_(sheet), formula_(std::move(formula))
        {

        }
        CellInterface::Value GetValue() const override;
        std::string GetText() const override;

    private:
        SheetInterface* sheet_;
        std::unique_ptr<FormulaInterface> formula_;
    };

    SheetInterface* sheet_ = nullptr;

    std::unique_ptr<Impl> impl_;

    bool is_valid_ = false;

    Value cache_;

    // €чейки, которые завис€т от this
    std::set<Position> back_cells_;

    // €чейки, от которых зависит this
    std::set<Position> next_cells_;

};



